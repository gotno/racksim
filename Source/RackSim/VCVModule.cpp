#include "VCVModule.h"

#include "osc3GameModeBase.h"
#include "osc3GameState.h"
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
#include "Utility/ModuleWeldment.h"

#include "Components/BoxComponent.h"
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
  StaticMeshComponent->SetCollisionResponseToChannel(WIDGET_TRACE, ECollisionResponse::ECR_Block);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> OutlineMeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  if (OutlineMeshBody.Object) OutlineMeshComponent->SetStaticMesh(OutlineMeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'"));
  if (FaceMaterial.Object) FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);

  // snap indicator meshes
  SnapIndicatorLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator Left"));
  SnapIndicatorLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorLeft->SetupAttachment(StaticMeshComponent);
  SnapIndicatorLeft->SetVisibility(false);
  SnapIndicatorLeftReflected = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator Left Reflected"));
  SnapIndicatorLeftReflected->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorLeftReflected->SetupAttachment(GetRootComponent());
  SnapIndicatorLeftReflected->SetVisibility(false);

  SnapIndicatorRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator Right"));
  SnapIndicatorRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorRight->SetupAttachment(StaticMeshComponent);
  SnapIndicatorRight->SetVisibility(false);
  SnapIndicatorRightReflected = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator Right Reflected"));
  SnapIndicatorRightReflected->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorRightReflected->SetupAttachment(GetRootComponent());
  SnapIndicatorRightReflected->SetVisibility(false);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> SnapIndicatorLeftMeshReference(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_snap_indicator_left.unit_snap_indicator_left'"));
  if (SnapIndicatorLeftMeshReference.Object) {
    SnapIndicatorLeft->SetStaticMesh(SnapIndicatorLeftMeshReference.Object);
    SnapIndicatorRightReflected->SetStaticMesh(SnapIndicatorLeftMeshReference.Object);
  }
  static ConstructorHelpers::FObjectFinder<UStaticMesh> SnapIndicatorRightMeshReference(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_snap_indicator_right.unit_snap_indicator_right'"));
  if (SnapIndicatorRightMeshReference.Object) {
    SnapIndicatorRight->SetStaticMesh(SnapIndicatorRightMeshReference.Object);
    SnapIndicatorLeftReflected->SetStaticMesh(SnapIndicatorRightMeshReference.Object);
  }

  // snap indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial> SnapIndicatorMaterial(TEXT("/Script/Engine.Material'/Game/materials/snap_indicator.snap_indicator'"));
  if (SnapIndicatorMaterial.Object) SnapIndicatorMaterialInterface = Cast<UMaterial>(SnapIndicatorMaterial.Object);

  // snap colliders
  SnapColliderLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("Snap Collider Left"));
  SnapColliderLeft->InitBoxExtent(FVector(0.5f, 0.005f, 0.5f));
  SnapColliderLeft->SetupAttachment(StaticMeshComponent);
  SnapColliderLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  SnapColliderLeft->SetCollisionObjectType(LEFT_SNAP_COLLIDER_OBJECT);

  SnapColliderRight = CreateDefaultSubobject<UBoxComponent>(TEXT("Snap Collider Right"));
  SnapColliderRight->InitBoxExtent(FVector(0.5f, 0.005f, 0.5f));
  SnapColliderRight->SetupAttachment(StaticMeshComponent);
  SnapColliderRight->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  SnapColliderRight->SetCollisionObjectType(RIGHT_SNAP_COLLIDER_OBJECT);

  // OutlineMaterialInterface setup in GrabbableActor

  // loading indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial>
    LoadingMaterialFinder(LoadingMaterialRef);
  if (LoadingMaterialFinder.Object)
    LoadingMaterialInterface = Cast<UMaterial>(LoadingMaterialFinder.Object);
}

void AVCVModule::BeginPlay() {
	Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  GameState = Cast<Aosc3GameState>(UGameplayStatics::GetGameState(this));

  // hide module until init'd
  SetHidden(true);

  UBodySetup* bodySetup = StaticMeshComponent->GetBodySetup();
  if (bodySetup) bodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;

  if (LoadingMaterialInterface) {
    LoadingMaterialInstance = UMaterialInstanceDynamic::Create(LoadingMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, LoadingMaterialInstance);
  }

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
  }

  if (SnapIndicatorMaterialInterface) {
    SnapIndicatorMaterialInstance = UMaterialInstanceDynamic::Create(SnapIndicatorMaterialInterface, this);
    SnapIndicatorLeft->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorLeftReflected->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorRight->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorRightReflected->SetMaterial(0, SnapIndicatorMaterialInstance);
  }

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
  UnscaledPanelWidth = Model.box.size.x;

  VCVOverrides overrides;
  FLinearColor bodyColor;
  if (!overrides.getBodyColor(Model.brand, bodyColor)) bodyColor = Model.bodyColor;

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), bodyColor);

  // initial values are UNSCALED
  StaticMeshComponent->SetWorldScale3D(FVector(UNSCALED_MODULE_DEPTH, UnscaledPanelWidth, UnscaledPanelHeight));
  OutlineMeshComponent->SetWorldScale3D(FVector(UNSCALED_MODULE_DEPTH, UnscaledPanelWidth, UnscaledPanelHeight));

  FVector snapIndicatorScale = FVector(UNSCALED_MODULE_DEPTH, 1, UnscaledPanelHeight);
  SnapIndicatorLeft->SetWorldScale3D(snapIndicatorScale);
  SnapIndicatorLeftReflected->SetWorldScale3D(snapIndicatorScale);
  SnapIndicatorRight->SetWorldScale3D(snapIndicatorScale);
  SnapIndicatorRightReflected->SetWorldScale3D(snapIndicatorScale);

  FVector meshLocation = StaticMeshComponent->GetComponentLocation();
  float halfWidth = UnscaledPanelWidth * 0.5f;
  FVector rightVector = StaticMeshComponent->GetRightVector();
  SnapIndicatorLeft->SetWorldLocation(meshLocation - rightVector * halfWidth);
  SnapIndicatorRight->SetWorldLocation(meshLocation + rightVector * halfWidth);

  float halfDepth = UNSCALED_MODULE_DEPTH * 0.5f;
  FVector forwardVector = StaticMeshComponent->GetForwardVector();
  SnapColliderLeft->SetWorldLocation(
    meshLocation - rightVector * halfWidth + forwardVector * halfDepth
  );
  SnapColliderRight->SetWorldLocation(
    meshLocation + rightVector * halfWidth + forwardVector * halfDepth
  );

  SpawnComponents();
  SetHidden(false);

  GameMode->RequestTexture(Model.panelSvgPath, this, FName("SetTexture"));

  Rescale();
  ReadyCallback();
}

void AVCVModule::SetTexture(FString& Filepath, UTexture2D* inTexture) {
  if (!Texture && Filepath.Equals(Model.panelSvgPath)) {
    Texture = inTexture;
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
  }
}

void AVCVModule::SetStagedForDestroy(bool inbStagedForDestroy) {
  bStagedForDestroy = inbStagedForDestroy;

  if (bStagedForDestroy) {
    for (auto& pair : ParamActors) pair.Value->SetActorHiddenInGame(true);
    for (auto& pair : LightActors) pair.Value->SetActorHiddenInGame(true);
    for (auto& pair : ParamLightActors) pair.Value->SetActorHiddenInGame(true);
    for (auto& pair : InputActors) pair.Value->SetActorHiddenInGame(true);
    for (auto& pair : OutputActors) pair.Value->SetActorHiddenInGame(true);
    FaceMaterialInstance->SetScalarParameterValue(FName("texture_alpha"), 0.f);
  } else {
    for (auto& pair : ParamActors) pair.Value->SetActorHiddenInGame(!pair.Value->Model->visible);
    for (auto& pair : LightActors) pair.Value->SetActorHiddenInGame(!pair.Value->Model->visible);
    for (auto& pair : ParamLightActors) pair.Value->SetActorHiddenInGame(!pair.Value->Model->visible);
    for (auto& pair : InputActors) pair.Value->SetActorHiddenInGame(!pair.Value->Model->visible);
    for (auto& pair : OutputActors) pair.Value->SetActorHiddenInGame(!pair.Value->Model->visible);
    FaceMaterialInstance->SetScalarParameterValue(FName("texture_alpha"), 1.f);
  }
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
  ContextMenu->Scale();
  ContextMenu->SetActorRotation(FRotator(0.f, 15.f, 0.f));

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
    // AVCVDisplay* a_display = GetWorld()->SpawnActor<AVCVDisplay>(
    //   AVCVDisplay::StaticClass(),
    //   GetActorLocation() + display.box.location(),
    //   FRotator(0, 0, 0),
    //   spawnParams
    // );
    // a_display->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    // a_display->Init(&display);
  }

  for (auto& pair : LightActors) pair.Value->HandleOverlap();
  for (auto& pair : ParamLightActors) pair.Value->HandleOverlap();
}

void AVCVModule::EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  if (!bHasBeenGrabbed) {
    bHasBeenGrabbed = true;
    OnFirstGrabbed.Broadcast(this);
  }

  Super::EngageGrab(GrabbedLocation, GrabbedRotation);
}

void AVCVModule::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  Super::AlterGrab(GrabbedLocation, GrabbedRotation);

  TriggerCableUpdates();
}

void AVCVModule::ReleaseGrab() {
  Super::ReleaseGrab();
  GameState->SetUnsaved();
  if (IsStagedForDestroy()) SetStagedForDestroy(false);
}

void AVCVModule::WeldSnap() {
  AVCVModule* snapToModule = Cast<AVCVModule>(SnapToSide->GetOwner());
  if (SnapToSide->GetCollisionObjectType() == LEFT_SNAP_COLLIDER_OBJECT) {
    GameMode->WeldModules(this, snapToModule, false);
  } else {
    GameMode->WeldModules(snapToModule, this, true);
  }

  SnapToSide = nullptr;
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

  SnapModeTick();

  // centers
  // DrawDebugSphere(
  //   GetWorld(),
  //   StaticMeshRoot->GetComponentLocation(),
  //   1.f,
  //   16,
  //   FColor::Blue
  // );
  // DrawDebugSphere(
  //   GetWorld(),
  //   StaticMeshComponent->GetComponentLocation(),
  //   1.1f,
  //   16,
  //   FColor::Red
  // );
  // DrawDebugSphere(
  //   GetWorld(),
  //   GetActorLocation(),
  //   2.f,
  //   16,
  //   IsInWeldment() ? FColor::Purple : FColor::Black
  // );

  // snap colliders
  // DrawDebugBox(
  //   GetWorld(),
  //   SnapColliderLeft->GetComponentLocation(),
  //   SnapColliderLeft->GetScaledBoxExtent(),
  //   SnapColliderLeft->GetComponentRotation().Quaternion(),
  //   FColor::Yellow
  // );
  // DrawDebugBox(
  //   GetWorld(),
  //   SnapColliderRight->GetComponentLocation(),
  //   SnapColliderRight->GetScaledBoxExtent(),
  //   SnapColliderRight->GetComponentRotation().Quaternion(),
  //   FColor::Red
  // );
}

void AVCVModule::InitSnapMode() {
  if (IsInWeldment()) {
    Weldment->InitSnapMode();
  } else {
    SetSnapMode(FSnapModeSide::Both);
  }
}

void AVCVModule::CancelSnapMode() {
  if (IsInWeldment()) {
    Weldment->CancelSnapMode();
  } else {
    SetSnapMode(FSnapModeSide::None);
  }
}

bool AVCVModule::IsInSnapMode(bool bActual) {
  if (IsInWeldment() && !bActual) return Weldment->IsInSnapMode();
  return SnapModeSide != FSnapModeSide::None;
};

void AVCVModule::SetSnapMode(FSnapModeSide inSnapModeSide) {
  SnapModeSide = inSnapModeSide;

  if (SnapModeSide == FSnapModeSide::None) {
    SnapIndicatorLeft->SetVisibility(false);
    SnapIndicatorLeftReflected->SetVisibility(false);
    SnapIndicatorRight->SetVisibility(false);
    SnapIndicatorRightReflected->SetVisibility(false);

    // toggling snap mode off, weld existing snap
    if (SnapToSide) WeldSnap();
  }
}

FHitResult AVCVModule::RunRightwardSnapTrace() {
  SnapIndicatorRight->SetVisibility(true);

  float halfWidth = GetPanelWidth() * 0.5f;
  FVector meshCenter =
    StaticMeshRoot->GetComponentLocation()
      + (StaticMeshRoot->GetForwardVector() * GetModuleDepth() * 0.5f);

  FVector traceStart =
    meshCenter + (StaticMeshRoot->GetRightVector() * (halfWidth + 0.1f));
  FVector traceEnd =
    traceStart + StaticMeshRoot->GetRightVector() * SnapTraceDistance * Scale;

  FHitResult rightwardHit;
  FCollisionObjectQueryParams queryParams;
  queryParams.AddObjectTypesToQuery(LEFT_SNAP_COLLIDER_OBJECT);
  GetWorld()->LineTraceSingleByObjectType(rightwardHit, traceStart, traceEnd, queryParams);
  // DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Yellow);

  // give snappable target the indicator
  if (rightwardHit.GetActor()) {
    AVCVModule* module = Cast<AVCVModule>(rightwardHit.GetActor());
    UBoxComponent* sideCollider = Cast<UBoxComponent>(rightwardHit.GetComponent());
    FVector location, vector;
    FRotator rotation;
    FSnapModeSide alignToSide =
      sideCollider->GetCollisionObjectType() == LEFT_SNAP_COLLIDER_OBJECT
        ? FSnapModeSide::Left
        : FSnapModeSide::Right;
    module->GetAlignToMeshInfo(alignToSide, location, vector, rotation);
    SnapIndicatorRightReflected->SetWorldLocation(location);
    SnapIndicatorRightReflected->SetWorldRotation(rotation);
    SnapIndicatorRightReflected->SetVisibility(true);
  } else {
    SnapIndicatorRightReflected->SetVisibility(false);
  }

  return rightwardHit;
}

FHitResult AVCVModule::RunLeftwardSnapTrace() {
  SnapIndicatorLeft->SetVisibility(true);

  float halfWidth = GetPanelWidth() * 0.5f;
  FVector meshCenter =
    StaticMeshRoot->GetComponentLocation()
      + (StaticMeshRoot->GetForwardVector() * GetModuleDepth() * 0.5f);

  FVector traceStart =
    meshCenter - (StaticMeshRoot->GetRightVector() * (halfWidth + 0.1f));
  FVector traceEnd =
    traceStart - StaticMeshRoot->GetRightVector() * SnapTraceDistance * Scale;

  FHitResult leftwardHit;
  FCollisionObjectQueryParams queryParams;
  queryParams.AddObjectTypesToQuery(RIGHT_SNAP_COLLIDER_OBJECT);
  GetWorld()->LineTraceSingleByObjectType(leftwardHit, traceStart, traceEnd, queryParams);
  DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Red);

  // give snappable target the indicator
  if (leftwardHit.GetActor()) {
    AVCVModule* module = Cast<AVCVModule>(leftwardHit.GetActor());
    UBoxComponent* sideCollider = Cast<UBoxComponent>(leftwardHit.GetComponent());
    FVector location, vector;
    FRotator rotation;
    FSnapModeSide alignToSide =
      sideCollider->GetCollisionObjectType() == LEFT_SNAP_COLLIDER_OBJECT
        ? FSnapModeSide::Left
        : FSnapModeSide::Right;
    module->GetAlignToMeshInfo(alignToSide, location, vector, rotation);
    SnapIndicatorLeftReflected->SetWorldLocation(location);
    SnapIndicatorLeftReflected->SetWorldRotation(rotation);
    SnapIndicatorLeftReflected->SetVisibility(true);
  } else {
    SnapIndicatorLeftReflected->SetVisibility(false);
  }

  return leftwardHit;
}

void AVCVModule::GetAlignToMeshInfo(FSnapModeSide AlignToSide, FVector& outLocation, FVector& outVector, FRotator& outRotation) {
  checkf(
    AlignToSide == FSnapModeSide::Left || AlignToSide == FSnapModeSide::Right,
    TEXT("must align with Left or Right side")
  );

  int direction = AlignToSide == FSnapModeSide::Left ? -1 : 1;
  float halfWidth = GetPanelWidth() * 0.5f;

  outVector = StaticMeshComponent->GetRightVector() * direction;
  outLocation = StaticMeshComponent->GetComponentLocation() + outVector * halfWidth;
  outRotation = StaticMeshComponent->GetComponentRotation();
}

void AVCVModule::SnapMesh() {
  checkf(!!SnapToSide, TEXT("cannot SnapMesh without SnapToSide"));

  AVCVModule* alignToModule = Cast<AVCVModule>(SnapToSide->GetOwner());
  FSnapModeSide alignToSide = 
    SnapToSide->GetCollisionObjectType() == LEFT_SNAP_COLLIDER_OBJECT
      ? FSnapModeSide::Left
      : FSnapModeSide::Right;
  AlignMeshTo(alignToModule, alignToSide, SnapIndicatorThickness);
  
  if (IsInWeldment()) Weldment->FollowSnap(this);
}

void AVCVModule::AlignMeshTo(AVCVModule* Module, FSnapModeSide AlignToSide, float Offset) {
  FVector edgeLocation, vector;
  FRotator rotation;
  Module->GetAlignToMeshInfo(AlignToSide, edgeLocation, vector, rotation);

  float halfWidth = GetPanelWidth() * 0.5f;
  FVector location = edgeLocation + vector * (GetPanelWidth() * 0.5f + Offset);
  StaticMeshComponent->SetWorldLocation(location);
  StaticMeshComponent->SetWorldRotation(rotation);
}

void AVCVModule::AlignActorTo(AVCVModule* Module, FSnapModeSide AlignToSide) {
  AlignMeshTo(Module, AlignToSide);
  CenterActorOnMesh();
}

void AVCVModule::SnapModeTick() {
  if (SnapModeSide == FSnapModeSide::None) return;

  FHitResult rightwardHit;
  if (SnapModeSide == FSnapModeSide::Both || SnapModeSide == FSnapModeSide::Right)
    rightwardHit = RunRightwardSnapTrace();
  FHitResult leftwardHit;
  if (SnapModeSide == FSnapModeSide::Both || SnapModeSide == FSnapModeSide::Left)
    leftwardHit = RunLeftwardSnapTrace();

  UBoxComponent* newSnapTo{nullptr};
  float SnapDistance = SnapTraceDistance * 0.75f;
  bool bCouldSnapRightward = rightwardHit.GetActor() && rightwardHit.Distance <= SnapDistance;
  bool bCouldSnapLeftward = leftwardHit.GetActor() && leftwardHit.Distance <= SnapDistance;
  if (bCouldSnapRightward && bCouldSnapLeftward) {
    float leftDistance = FVector::Dist(
      rightwardHit.GetComponent()->GetComponentLocation(),
      StaticMeshRoot->GetComponentLocation()
    );
    float rightDistance = FVector::Dist(
      rightwardHit.GetComponent()->GetComponentLocation(),
      StaticMeshRoot->GetComponentLocation()
    );
    if (leftDistance < rightDistance) {
      newSnapTo = Cast<UBoxComponent>(rightwardHit.GetComponent());
      SnapIndicatorRight->SetVisibility(false);
    } else {
      newSnapTo = Cast<UBoxComponent>(leftwardHit.GetComponent());
      SnapIndicatorLeft->SetVisibility(false);
    }
  } else if (bCouldSnapRightward) {
    newSnapTo = Cast<UBoxComponent>(rightwardHit.GetComponent());
    SnapIndicatorLeft->SetVisibility(false);
  } else if (bCouldSnapLeftward) {
    newSnapTo = Cast<UBoxComponent>(leftwardHit.GetComponent());
    SnapIndicatorRight->SetVisibility(false);
  }
  
  if (newSnapTo != SnapToSide) {
    SnapToSide = newSnapTo;
    if (!SnapToSide) {
      ResetMeshPosition();
      if (IsInWeldment()) Weldment->FollowSnap(nullptr);
    }
  }

  if (SnapToSide) SnapMesh();
}

void AVCVModule::GetModulePosition(FVector& Location, FRotator& Rotation) {
  Location = StaticMeshComponent->GetComponentLocation();
  Rotation = StaticMeshComponent->GetComponentRotation();
}

void AVCVModule::GetModuleLandingPosition(FVector& Location, FRotator& Rotation, bool bOffset) {
  GetModulePosition(Location, Rotation);
  if (bOffset) Location -= StaticMeshComponent->GetForwardVector() * 2 * GetModuleDepth();
}

void AVCVModule::Rescale() {
  SetActorScale3D(FVector(Scale));
  ContextMenu->Scale();
}