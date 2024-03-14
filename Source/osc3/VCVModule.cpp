#include "VCVModule.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "CableEnd.h"
#include "VCVData/VCV.h"
#include "VCVData/VCVOverrides.h"
#include "ModuleComponents/ContextMenu.h"
#include "ModuleComponents/VCVLight.h"
#include "ModuleComponents/VCVKnob.h"
#include "ModuleComponents/VCVButton.h"
#include "ModuleComponents/VCVSwitch.h"
#include "ModuleComponents/VCVSlider.h"
#include "ModuleComponents/VCVPort.h"
#include "ModuleComponents/VCVDisplay.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodySetup.h"

AVCVModule::AVCVModule() {
	PrimaryActorTick.bCanEverTick = true;

  // RootSceneComponent/StaticMeshComponent/OutlineMeshComponent setup in GrabbableActor
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_module_faced.unit_module_faced'"));
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> OutlineMeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  if (OutlineMeshBody.Object) OutlineMeshComponent->SetStaticMesh(OutlineMeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'"));
  if (FaceMaterial.Object) FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);

  // OutlineMaterialInterface setup in GrabbableActor
}

void AVCVModule::BeginPlay() {
	Super::BeginPlay();
  
  // hide module until init'd
  SetHidden(true);

  UBodySetup* bodySetup = StaticMeshComponent->GetBodySetup();
  if (bodySetup) bodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
  Tags.Add(TAG_INTERACTABLE);
}

void AVCVModule::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

  for (auto& pair : InputActors) {
    AVCVPort* port = pair.Value;
    while (port->HasConnections()) {
      ACableEnd* cableEnd = port->GetTopCableEnd();
      port->Disconnect(cableEnd);
      cableEnd->HandleDisconnected();
      GameMode->DestroyCableActor(cableEnd->Cable);
    }
  }

  for (auto& pair : OutputActors) {
    AVCVPort* port = pair.Value;
    while (port->HasConnections()) {
      ACableEnd* cableEnd = port->GetTopCableEnd();
      port->Disconnect(cableEnd);
      cableEnd->HandleDisconnected();
      GameMode->DestroyCableActor(cableEnd->Cable);
    }
  }

  TArray<AActor*, FDefaultAllocator> attachedActors;
  GetAttachedActors(attachedActors, true, true);
  for (AActor* attachedActor : attachedActors) {
    attachedActor->Destroy();
  }
}

void AVCVModule::Init(VCVModule vcv_module, TFunction<void ()> ReadyCallback) {
  Model = vcv_module; 

  Id = Model.id;
  Brand = Model.brand;
  Name = Model.name;

  VCVOverrides overrides;

  FLinearColor bodyColor;
  if (!overrides.getBodyColor(Model.brand, bodyColor)) bodyColor = Model.bodyColor;

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), bodyColor);

  FaceMaterialInstance->SetScalarParameterValue(FName("uscale"), overrides.getUVOverride(Model.brand).X);
  FaceMaterialInstance->SetScalarParameterValue(FName("vscale"), overrides.getUVOverride(Model.brand).Y);

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, Model.box.size.x, Model.box.size.y));
  OutlineMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH + 0.2f, Model.box.size.x + 0.2f, Model.box.size.y + 0.2f));

  SpawnComponents();
  SetHidden(false);

  ReadyCallback();
}

void AVCVModule::GetSlugs(FString& PluginSlug, FString& Slug) {
  PluginSlug = Model.pluginSlug;
  Slug = Model.slug;
}

AVCVPort* AVCVModule::GetPortActor(PortType Type, int32& PortId) {
  if (Type == PortType::Input) return InputActors[PortId];
  return OutputActors[PortId];
}

void AVCVModule::UpdateLight(int32 LightId, FLinearColor Color) {
  AVCVLight* light{nullptr};
  if (LightActors.Contains(LightId)) {
    light = LightActors[LightId];
  } else if (ParamLightActors.Contains(LightId)) {
    light = ParamLightActors[LightId];
  }

  if (light) {
    light->SetEmissiveColor(Color);
    light->SetEmissiveIntensity(Color.A);
  }
}
void AVCVModule::RegisterParamLight(int64_t LightId, AVCVLight* LightActor) {
  ParamLightActors.Add(LightId, LightActor);
}

void AVCVModule::ParamUpdated(int32 ParamId, float Value) {
  if (!GameMode) return;
  GameMode->SendParamUpdate(Model.id, ParamId, Value);
}

void AVCVModule::SpawnComponents() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  ContextMenu = GetWorld()->SpawnActor<AContextMenu>(
    AContextMenu::StaticClass(),
    GetActorLocation(),
    FRotator(0, 0, 0),
    spawnParams
  );
  ContextMenu->AttachToComponent(
    StaticMeshComponent,
    FAttachmentTransformRules::KeepWorldTransform
  );
  ContextMenu->Init(Model.box.size);

  for (TPair<int32, VCVParam>& param_kvp : Model.Params) {
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
      aParam->Init(&param);
      ParamActors.Add(param.id, aParam);
    } else {
      UE_LOG(LogTemp, Warning, TEXT("param actor init failure! %d:%d"), Model.id, param.id);
    }
  }

  for (TPair<int32, VCVPort>& port_kvp : Model.Inputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->Init(&port);
    InputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVPort>& port_kvp : Model.Outputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->Init(&port);
    OutputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVLight>& light_kvp : Model.Lights) {
    VCVLight& light = light_kvp.Value;

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      GetActorLocation() + light.box.location(),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_light->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_light->Init(&light);
    LightActors.Add(light.id, a_light);
  }

  for (VCVDisplay& display : Model.Displays) {
    AVCVDisplay* a_display = GetWorld()->SpawnActor<AVCVDisplay>(
      AVCVDisplay::StaticClass(),
      GetActorLocation() + display.box.location(),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_display->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_display->Init(&display);
  }
}

void AVCVModule::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  Super::AlterGrab(GrabbedLocation, GrabbedRotation);

  TriggerCableUpdates();
}

void AVCVModule::TriggerCableUpdates() {
  for (auto& pair : InputActors) pair.Value->TriggerCableUpdates();
  for (auto& pair : OutputActors) pair.Value->TriggerCableUpdates();
}

void AVCVModule::ToggleContextMenu() {
  ContextMenu->ToggleVisible();
}

void AVCVModule::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (Texture) return;

  Texture = GameMode->GetTexture(Model.panelSvgPath);
  if (Texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
}

void AVCVModule::GetModulePosition(FVector& Location, FRotator& Rotation) {
  Location = StaticMeshComponent->GetComponentLocation();
  Rotation = StaticMeshComponent->GetComponentRotation();
}

void AVCVModule::GetModuleLandingPosition(FVector& Location, FRotator& Rotation, bool bOffset) {
  GetModulePosition(Location, Rotation);
  if (bOffset) Location -= StaticMeshComponent->GetForwardVector() * (2 * MODULE_DEPTH * RENDER_SCALE);
}