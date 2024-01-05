#include "VCVModule.h"

#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVOverrides.h"
#include "VCVLight.h"
#include "VCVKnob.h"
#include "VCVButton.h"
#include "VCVSwitch.h"
#include "VCVSlider.h"
#include "VCVPort.h"
#include "VCVDisplay.h"
#include "ui/ContextMenu.h"
#include "ui/ContextMenuEntryData.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "Algo/Reverse.h"

AVCVModule::AVCVModule() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetupAttachment(GetRootComponent());
  StaticMeshComponent->SetRenderInDepthPass(true);
  StaticMeshComponent->SetRenderCustomDepth(true);
  StaticMeshComponent->SetCustomDepthStencilValue(2);

  OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Outline Mesh"));
  OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  OutlineMeshComponent->SetupAttachment(StaticMeshComponent);
  OutlineMeshComponent->SetVisibility(false);
  OutlineMeshComponent->SetWorldScale3D(FVector(1.1f, 1.1f, 1.1f));
  OutlineMeshComponent->AddLocalOffset(FVector(-0.05f, 0.f, 0.f));
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_module_faced.unit_module_faced'"));
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> OutlineMeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  if (OutlineMeshBody.Object) OutlineMeshComponent->SetStaticMesh(OutlineMeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'"));
  if (FaceMaterial.Object) FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> OutlineMaterial(TEXT("/Script/Engine.Material'/Game/materials/looman_outlines/M_LocalOutlines.M_LocalOutlines'"));
  if (OutlineMaterial.Object) OutlineMaterialInterface = Cast<UMaterial>(OutlineMaterial.Object);
  
  ContextMenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ContextMenuWidget"));
  ContextMenuWidgetComponent->SetWindowFocusable(false);

  static ConstructorHelpers::FClassFinder<UContextMenu>
    contextMenuWidgetObject(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_ContextMenu.BP_ContextMenu_C'"));
  if (contextMenuWidgetObject.Succeeded()) {
    ContextMenuWidgetComponent->SetWidgetClass(contextMenuWidgetObject.Class);
  }
}

void AVCVModule::BeginPlay() {
	Super::BeginPlay();
  
  // hide module until init'd
  SetHidden(true);

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }

  if (OutlineMaterialInterface) {
    OutlineMaterialInstance = UMaterialInstanceDynamic::Create(OutlineMaterialInterface, this);
    OutlineMeshComponent->SetMaterial(0, OutlineMaterialInstance);
  }
  
  ContextMenuWidget = Cast<UContextMenu>(ContextMenuWidgetComponent->GetUserWidgetObject());
  ContextMenuWidgetComponent->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
  ContextMenuWidgetComponent->SetVisibility(false);

  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
  Tags.Add(TAG_INTERACTABLE);
  Tags.Add(TAG_GRABBABLE);
}

void AVCVModule::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
  
  int64 cableId;
  for (auto& pair : InputActors) {
    while (pair.Value->getCableId(cableId)) {
      gameMode->DestroyCableActor(cableId);
    }
  }

  for (auto& pair : OutputActors) {
    while (pair.Value->getCableId(cableId)) {
      gameMode->DestroyCableActor(cableId);
    }
  }

  TArray<AActor*, FDefaultAllocator> attachedActors;
  GetAttachedActors(attachedActors, true, true);
  for (AActor* attachedActor : attachedActors) {
    attachedActor->Destroy();
  }
}

void AVCVModule::init(VCVModule vcv_module) {
  model = vcv_module; 
  Brand = model.brand;
  Name = model.name;

  VCVOverrides overrides;

  FLinearColor bodyColor;
  if (!overrides.getBodyColor(model.brand, bodyColor)) bodyColor = model.bodyColor;

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), bodyColor);

  FaceMaterialInstance->SetScalarParameterValue(FName("uscale"), overrides.getUVOverride(model.brand).X);
  FaceMaterialInstance->SetScalarParameterValue(FName("vscale"), overrides.getUVOverride(model.brand).Y);

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, model.box.size.x, model.box.size.y));
  spawnComponents();
  SetHidden(false);
  
  SetupContextMenuWidget();

  SetActorRotation(FRotator(0.f, 0.f, 0.f));
}

void AVCVModule::SetupContextMenuWidget() {
  FVector2D drawSize(350.f, 700.f);
  float desiredContextMenuHeight = model.box.size.y - 2 * RENDER_SCALE;
  float scale = desiredContextMenuHeight / drawSize.Y;

  ContextMenuWidgetComponent->SetDrawSize(drawSize);
  ContextMenuWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));

  float halfModuleWidth = model.box.size.x * 0.5f;
  float halfMenuWidth = scale * drawSize.X * 0.5f;
  float rightOffset = halfModuleWidth + halfMenuWidth;
  ContextMenuWidgetComponent->SetWorldLocation(
    GetActorLocation()
    + (GetActorRightVector() * rightOffset)
    + (GetActorForwardVector() * -1.f)
  );

  ContextMenuWidgetComponent->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
}

void AVCVModule::GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector) {
  AVCVPort* port;
  if (identity.type == PortType::Input) {
    port = InputActors[identity.portId];
  } else {
    port = OutputActors[identity.portId];
  }
  portLocation = port->GetActorLocation();
  portForwardVector = port->GetActorForwardVector();
}

int64 AVCVModule::GetId() {
  return model.id;
}

void AVCVModule::GetSlugs(FString& PluginSlug, FString& Slug) {
  PluginSlug = model.pluginSlug;
  Slug = model.slug;
}

AVCVPort* AVCVModule::GetPortActor(PortIdentity identity) {
  if (identity.type == PortType::Input) return InputActors[identity.portId];
  return OutputActors[identity.portId];
}

void AVCVModule::AttachCable(const PortIdentity& identity, int64_t cableId) {
  if (identity.type == PortType::Input) {
    InputActors[identity.portId]->addCableId(cableId);
  } else {
    OutputActors[identity.portId]->addCableId(cableId);
  }
}

void AVCVModule::DetachCable(const PortIdentity& identity, int64_t cableId) {
  if (identity.type == PortType::Input) {
    InputActors[identity.portId]->removeCableId(cableId);
  } else {
    OutputActors[identity.portId]->removeCableId(cableId);
  }
}

void AVCVModule::UpdateLight(int32 lightId, FLinearColor color) {
  if (LightActors.Contains(lightId)) {
    LightActors[lightId]->SetEmissiveColor(color);
    LightActors[lightId]-> SetEmissiveIntensity(color.A);
  } else if (ParamLightActors.Contains(lightId)) {
    ParamLightActors[lightId]->SetEmissiveColor(color);
    ParamLightActors[lightId]-> SetEmissiveIntensity(color.A);
  }
}
void AVCVModule::registerParamLight(int64_t lightId, AVCVLight* lightActor) {
  ParamLightActors.Add(lightId, lightActor);
}

void AVCVModule::paramUpdated(int32 paramId, float value) {
  if (!gameMode) return;
  gameMode->SendParamUpdate(model.id, paramId, value);
}

void AVCVModule::spawnComponents() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  for (TPair<int32, VCVParam>& param_kvp : model.Params) {
    VCVParam& param = param_kvp.Value;
    AVCVParam* aParam = nullptr;

    if (param.type == ParamType::Knob) {
      aParam = GetWorld()->SpawnActor<AVCVKnob>(
        AVCVKnob::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
    } else if (param.type == ParamType::Slider) {
      aParam = GetWorld()->SpawnActor<AVCVSlider>(
        AVCVSlider::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
    } else if (param.type == ParamType::Switch) {
      aParam = GetWorld()->SpawnActor<AVCVSwitch>(
        AVCVSwitch::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
    } else if (param.type == ParamType::Button) {
      aParam = GetWorld()->SpawnActor<AVCVButton>(
        AVCVButton::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
    }

    if (aParam) {
      aParam->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
      aParam->init(&param);
      ParamActors.Add(param.id, aParam);
    } else {
      UE_LOG(LogTemp, Warning, TEXT("param actor init failure! %d:%d"), model.id, param.id);
    }
  }

  for (TPair<int32, VCVPort>& port_kvp : model.Inputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->init(&port);
    InputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVPort>& port_kvp : model.Outputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->init(&port);
    OutputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVLight>& light_kvp : model.Lights) {
    VCVLight& light = light_kvp.Value;

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      GetActorLocation() + light.box.location(),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_light->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_light->init(&light);
    LightActors.Add(light.id, a_light);
  }

  for (VCVDisplay& display : model.Displays) {
    AVCVDisplay* a_display = GetWorld()->SpawnActor<AVCVDisplay>(
      AVCVDisplay::StaticClass(),
      GetActorLocation() + display.box.location(),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_display->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_display->init(&display);
  }
}

void AVCVModule::SetHighlighted(bool bHighlighted, FLinearColor HighlightColor) {
  OutlineMeshComponent->SetVisibility(bHighlighted);
  OutlineMaterialInstance->SetVectorParameterValue(FName("Color"), HighlightColor);
}

void AVCVModule::EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab engage"), *GetActorNameOrLabel());
  bGrabEngaged = true;
  GrabOffset = GrabbedLocation - GetActorLocation();
  StaticMeshComponent->AddWorldOffset(-GrabOffset);

  LastGrabbedRotation = GrabbedRotation;
  LastGrabbedLocation = GrabbedLocation - GrabOffset;
  LastLocationDelta = FVector(0.f, 0.f, 0.f);
  SetHighlighted(false);
}

void AVCVModule::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  FQuat qFrom = LastGrabbedRotation.Quaternion();
  FQuat qTo =  GrabbedRotation.Quaternion();
  FQuat qDelta = qTo * qFrom.Inverse();
  
  FVector locationDelta = GrabbedLocation - LastGrabbedLocation;
  locationDelta = UKismetMathLibrary::WeightedMovingAverage_FVector(
    locationDelta,
    LastLocationDelta,
    0.4f
  );
  LastLocationDelta = locationDelta;

  AddActorWorldOffset(locationDelta);
  AddActorWorldRotation(qDelta);

  LastGrabbedLocation = GrabbedLocation;
  LastGrabbedRotation = GrabbedRotation;
}

void AVCVModule::ReleaseGrab() {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab release"), *GetActorNameOrLabel());
  bGrabEngaged = false;
  StaticMeshComponent->AddWorldOffset(GrabOffset);
  AddActorWorldOffset(-GrabOffset);
}

void AVCVModule::ToggleContextMenu() {
  if (ContextMenuWidgetComponent->IsVisible()) {
    CloseMenu();
    return;
  }

  // (re)create the base menu struct
  MakeMenu();

  ContextMenuWidgetComponent->SetVisibility(true);
}

void AVCVModule::MakeMenu(int ParentMenuId, int ParentItemIndex) {
  int foundMenuIndex = ContextMenus.IndexOfByPredicate([&](const VCVMenu& menu) {
    return menu.parentMenuId == ParentMenuId && menu.parentItemIndex == ParentItemIndex;
  });

  if (foundMenuIndex != INDEX_NONE) {
    RequestMenu(foundMenuIndex);
    return;
  }

  int menuId = ContextMenus.Num();
  VCVMenu menu(model.id, menuId);

  menu.parentMenuId = ParentMenuId;
  menu.parentItemIndex = ParentItemIndex;

  ContextMenus.Push(menu);
  RequestMenu(menuId);
}

void AVCVModule::RequestMenu(int MenuId) {
  gameMode->RequestMenu(ContextMenus[MenuId]);
}

FString AVCVModule::MakeMenuBreadcrumbs(int MenuId) {
  TArray<FString> crumbs;

  int currentId = MenuId;
  while (currentId > 0) {
    int parentMenuId = ContextMenus[currentId].parentMenuId;
    int parentItemIndex = ContextMenus[currentId].parentItemIndex;

    crumbs.Push(ContextMenus[parentMenuId].MenuItems[parentItemIndex].text);

    currentId = parentMenuId;
  }

  Algo::Reverse(crumbs);
  return FString::Join(crumbs, _T(" > "));
}

void AVCVModule::SetMenu(int MenuId) {
  TArray<UContextMenuEntryData*> entries;

  // add close/back button
  UContextMenuEntryData* backButtonEntry =
    NewObject<UContextMenuEntryData>(this);
  backButtonEntry->MenuItem.type = VCVMenuItemType::BACK;
  backButtonEntry->MenuItem.text = MenuId == 0 ? FString("Close") : FString("Back");
  backButtonEntry->Module = this;
  backButtonEntry->ParentMenuId = ContextMenus[MenuId].parentMenuId;
  backButtonEntry->DividerNext = true;
  entries.Add(backButtonEntry);

  // add breadcrumbs label for submenus
  if (MenuId != 0) {
    int parentMenuId = ContextMenus[MenuId].parentMenuId;
    int parentItemIndex = ContextMenus[MenuId].parentItemIndex;

    UContextMenuEntryData* crumbsLabelEntry =
      NewObject<UContextMenuEntryData>(this);
    crumbsLabelEntry->MenuItem.type = VCVMenuItemType::LABEL;
    crumbsLabelEntry->MenuItem.text = MakeMenuBreadcrumbs(MenuId);
    crumbsLabelEntry->DividerNext = true;
    entries.Add(crumbsLabelEntry);
  }

  for (auto& pair : ContextMenus[MenuId].MenuItems) {
    VCVMenuItem& menuItem = pair.Value;
    VCVMenuItemType divider = VCVMenuItemType::DIVIDER;
    if (menuItem.type == divider) continue;

    UContextMenuEntryData* entry = NewObject<UContextMenuEntryData>(this);
    entry->MenuItem = menuItem;
    entry->Module = this;

    int& index = pair.Key;
    if (ContextMenus[MenuId].MenuItems.Contains(index + 1)) 
      if (ContextMenus[MenuId].MenuItems[index + 1].type == divider) 
        entry->DividerNext = true;

    entries.Add(entry);
  }

  ContextMenuWidget->SetListItems(entries);
}

void AVCVModule::CloseMenu() {
  ContextMenuWidgetComponent->SetVisibility(false);
  ContextMenus.Empty();
}

void AVCVModule::AddMenuItem(VCVMenuItem& MenuItem) {
  ContextMenus[MenuItem.menuId].MenuItems.Add(MenuItem.index, MenuItem);
}

void AVCVModule::MenuSynced(VCVMenu& Menu) {
  ContextMenus[Menu.id].MenuItems.KeySort([](int A, int B) { return A < B; });
  SetMenu(Menu.id);
}

void AVCVModule::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (texture) return;

  texture = gameMode->GetTexture(model.panelSvgPath);
  if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
}