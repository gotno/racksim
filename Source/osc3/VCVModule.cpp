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
  SnapIndicatorExternalRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator External Right"));
  SnapIndicatorExternalRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorExternalRight->SetupAttachment(GetRootComponent());
  SnapIndicatorExternalRight->SetVisibility(false);

  SnapIndicatorRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator Right"));
  SnapIndicatorRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorRight->SetupAttachment(StaticMeshComponent);
  SnapIndicatorRight->SetVisibility(false);
  SnapIndicatorExternalLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Snap Indicator External Left"));
  SnapIndicatorExternalLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  SnapIndicatorExternalLeft->SetupAttachment(GetRootComponent());
  SnapIndicatorExternalLeft->SetVisibility(false);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> SnapIndicatorLeftMeshReference(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_snap_indicator_left.unit_snap_indicator_left'"));
  if (SnapIndicatorLeftMeshReference.Object) {
    SnapIndicatorLeft->SetStaticMesh(SnapIndicatorLeftMeshReference.Object);
    SnapIndicatorExternalLeft->SetStaticMesh(SnapIndicatorLeftMeshReference.Object);
  }
  static ConstructorHelpers::FObjectFinder<UStaticMesh> SnapIndicatorRightMeshReference(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_snap_indicator_right.unit_snap_indicator_right'"));
  if (SnapIndicatorRightMeshReference.Object) {
    SnapIndicatorRight->SetStaticMesh(SnapIndicatorRightMeshReference.Object);
    SnapIndicatorExternalRight->SetStaticMesh(SnapIndicatorRightMeshReference.Object);
  }

  // snap indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial> SnapIndicatorMaterial(TEXT("/Script/Engine.Material'/Game/materials/snap_indicator.snap_indicator'"));
  if (SnapIndicatorMaterial.Object) SnapIndicatorMaterialInterface = Cast<UMaterial>(SnapIndicatorMaterial.Object);

  // snap colliders
  SnapColliderLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("Snap Collider Left"));
  SnapColliderLeft->InitBoxExtent(FVector(0.5f, 0.005f, 0.5f));
  SnapColliderLeft->AddWorldOffset(StaticMeshComponent->GetForwardVector() * 0.5f);
  SnapColliderLeft->SetupAttachment(StaticMeshComponent);
  SnapColliderLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  SnapColliderLeft->SetCollisionObjectType(LEFT_SNAP_OBJECT);

  SnapColliderRight = CreateDefaultSubobject<UBoxComponent>(TEXT("Snap Collider Right"));
  SnapColliderRight->InitBoxExtent(FVector(0.5f, 0.005f, 0.5f));
  SnapColliderRight->AddWorldOffset(StaticMeshComponent->GetForwardVector() * 0.5f);
  SnapColliderRight->SetupAttachment(StaticMeshComponent);
  SnapColliderRight->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  SnapColliderRight->SetCollisionObjectType(RIGHT_SNAP_OBJECT);

  // OutlineMaterialInterface setup in GrabbableActor
}

void AVCVModule::BeginPlay() {
	Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
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

  if (SnapIndicatorMaterialInterface) {
    SnapIndicatorMaterialInstance = UMaterialInstanceDynamic::Create(SnapIndicatorMaterialInterface, this);
    SnapIndicatorLeft->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorExternalRight->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorRight->SetMaterial(0, SnapIndicatorMaterialInstance);
    SnapIndicatorExternalLeft->SetMaterial(0, SnapIndicatorMaterialInstance);
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

  VCVOverrides overrides;

  FLinearColor bodyColor;
  if (!overrides.getBodyColor(Model.brand, bodyColor)) bodyColor = Model.bodyColor;

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), bodyColor);

  FaceMaterialInstance->SetScalarParameterValue(FName("uscale"), overrides.getUVOverride(Model.brand).X);
  FaceMaterialInstance->SetScalarParameterValue(FName("vscale"), overrides.getUVOverride(Model.brand).Y);

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, Model.box.size.x, Model.box.size.y));
  OutlineMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH + 0.2f, Model.box.size.x + 0.2f, Model.box.size.y + 0.2f));

  float halfWidth = Model.box.size.x * 0.5f;
  SnapColliderLeft->AddWorldOffset(-StaticMeshComponent->GetRightVector() * halfWidth);
  SnapColliderRight->AddWorldOffset(StaticMeshComponent->GetRightVector() * halfWidth);

  SnapIndicatorLeft->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, 1, Model.box.size.y));
  SnapIndicatorExternalRight->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, 1, Model.box.size.y));
  SnapIndicatorLeft->AddWorldOffset(-StaticMeshComponent->GetRightVector() * halfWidth);
  SnapIndicatorRight->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, 1, Model.box.size.y));
  SnapIndicatorExternalLeft->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, 1, Model.box.size.y));
  SnapIndicatorRight->AddWorldOffset(StaticMeshComponent->GetRightVector() * halfWidth);

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

void AVCVModule::WeldSnap() {
  AVCVModule* snapToModule = Cast<AVCVModule>(SnapToSide->GetOwner());
  if (SnapToSide->GetCollisionObjectType() == LEFT_SNAP_OBJECT) {
    GameMode->WeldModules(this, snapToModule);
  } else {
    GameMode->WeldModules(snapToModule, this);
  }

  SnapToSide = nullptr;
}

void AVCVModule::ReleaseGrab() {
  Super::ReleaseGrab();

  if (SnapToSide) {
    ResetMeshPosition();

    FVector offset;
    FRotator rotation;
    GetSnapOffset(SnapToSide, offset, rotation);
    SetActorRotation(rotation);
    AddActorWorldOffset(offset);

    WeldSnap();
  } else if (IsInWeldment()) {
    Weldment->ReleaseGrab();
  }
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
  
  if (!Texture) {
    Texture = GameMode->GetTexture(Model.panelSvgPath);
    if (Texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
  }

  SnapModeTick();
  if (SnapToSide) {
    FVector offset;
    FRotator rotation;
    GetSnapOffset(SnapToSide, offset, rotation);
    OffsetMesh(offset, rotation);
  }

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

void AVCVModule::SetSnapMode(bool inbSnapMode) {
  bSnapMode = inbSnapMode;

  SnapIndicatorLeft->SetVisibility(bSnapMode);
  SnapIndicatorRight->SetVisibility(bSnapMode);

  if (!bSnapMode) {
    SnapIndicatorExternalLeft->SetVisibility(false);
    SnapIndicatorExternalRight->SetVisibility(false);

    // weld existing snap if we're toggling snap mode off
    if (SnapToSide) {
      ResetMeshPosition();

      FVector offset;
      FRotator rotation;
      GetSnapOffset(SnapToSide, offset, rotation);
      OffsetMesh(offset, rotation);

      WeldSnap();
    }
  }
}

FHitResult AVCVModule::RunLeftSnapTrace() {
  float halfWidth = Model.box.size.x * 0.5f;
  FVector meshCenter =
    StaticMeshRoot->GetComponentLocation()
      + (StaticMeshRoot->GetForwardVector() * MODULE_DEPTH * RENDER_SCALE * 0.5f);

  FVector traceStart =
    meshCenter + (StaticMeshRoot->GetRightVector() * (halfWidth + 0.1f));
  FVector traceEnd =
    traceStart + StaticMeshRoot->GetRightVector() * SnapTraceDistance * RENDER_SCALE;

  FHitResult leftHit;
  FCollisionObjectQueryParams leftQueryParams;
  leftQueryParams.AddObjectTypesToQuery(LEFT_SNAP_OBJECT);
  GetWorld()->LineTraceSingleByObjectType(leftHit, traceStart, traceEnd, leftQueryParams);
  // DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Yellow);

  // give snappable target the indicator
  if (leftHit.GetActor()) {
    AVCVModule* module = Cast<AVCVModule>(leftHit.GetActor());
    UBoxComponent* sideCollider = Cast<UBoxComponent>(leftHit.GetComponent());
    FVector location, vector;
    FRotator rotation;
    module->GetSnapPositioning(sideCollider, location, vector, rotation);
    SnapIndicatorExternalLeft->SetWorldLocation(location);
    SnapIndicatorExternalLeft->SetWorldRotation(rotation);
    SnapIndicatorExternalLeft->SetVisibility(true);
  } else {
    SnapIndicatorExternalLeft->SetVisibility(false);
  }

  return leftHit;
}

FHitResult AVCVModule::RunRightSnapTrace() {
  float halfWidth = Model.box.size.x * 0.5f;
  FVector meshCenter =
    StaticMeshRoot->GetComponentLocation()
      + (StaticMeshRoot->GetForwardVector() * MODULE_DEPTH * RENDER_SCALE * 0.5f);

  FVector traceStart =
    meshCenter - (StaticMeshRoot->GetRightVector() * (halfWidth + 0.1f));
  FVector traceEnd =
    traceStart - StaticMeshRoot->GetRightVector() * SnapTraceDistance * RENDER_SCALE;

  FHitResult rightHit;
  FCollisionObjectQueryParams rightQueryParams;
  rightQueryParams.AddObjectTypesToQuery(RIGHT_SNAP_OBJECT);
  GetWorld()->LineTraceSingleByObjectType(rightHit, traceStart, traceEnd, rightQueryParams);
  // DrawDebugLine(GetWorld(), traceStart, traceEnd, FColor::Red);

  // give snappable target the indicator
  if (rightHit.GetActor()) {
    AVCVModule* module = Cast<AVCVModule>(rightHit.GetActor());
    UBoxComponent* sideCollider = Cast<UBoxComponent>(rightHit.GetComponent());
    FVector location, vector;
    FRotator rotation;
    module->GetSnapPositioning(sideCollider, location, vector, rotation);
    SnapIndicatorExternalRight->SetWorldLocation(location);
    SnapIndicatorExternalRight->SetWorldRotation(rotation);
    SnapIndicatorExternalRight->SetVisibility(true);
  } else {
    SnapIndicatorExternalRight->SetVisibility(false);
  }

  return rightHit;
}

void AVCVModule::GetSnapPositioning(UBoxComponent* SideCollider, FVector& OffsetLocation, FVector& Vector, FRotator& Rotation) {
  int direction = SideCollider == SnapColliderLeft ? -1 : 1;
  float halfWidth = Model.box.size.x * 0.5f;

  Rotation = StaticMeshComponent->GetComponentRotation();
  Vector = direction * StaticMeshComponent->GetRightVector();
  OffsetLocation = StaticMeshComponent->GetComponentLocation() + Vector * halfWidth;
}

void AVCVModule::GetSnapOffset(UBoxComponent* SideCollider, FVector& Offset, FRotator& Rotation) {
  AVCVModule* module = Cast<AVCVModule>(SideCollider->GetOwner());
  FVector location, vector;
  FRotator rotation;
  module->GetSnapPositioning(SideCollider, location, vector, rotation);

  Rotation = rotation;
  FVector newLocation = location + vector * Model.box.size.x * 0.5f;
  Offset = newLocation - StaticMeshRoot->GetComponentLocation();
}

void AVCVModule::OffsetMesh(FVector Offset, FRotator Rotation) {
  ResetMeshPosition();
  StaticMeshComponent->AddWorldOffset(Offset);
  StaticMeshComponent->SetWorldRotation(Rotation);
}

void AVCVModule::ResetMeshPosition() {
  StaticMeshComponent->SetWorldRotation(StaticMeshRoot->GetComponentRotation());
  StaticMeshComponent->SetWorldLocation(StaticMeshRoot->GetComponentLocation());
}

void AVCVModule::SnapModeTick() {
  if (!bSnapMode) return;
  if (IsInWeldment()) {
    Weldment->SnapModeTick();
    return;
  }

  FHitResult leftHit = RunLeftSnapTrace();
  FHitResult rightHit = RunRightSnapTrace();

  UBoxComponent* newSnapTo{nullptr};
  float SnapDistance = SnapTraceDistance * 0.6f;
  bool bHasLeftHit = leftHit.GetActor() && leftHit.Distance <= SnapDistance;
  bool bHasRightHit = rightHit.GetActor() && rightHit.Distance <= SnapDistance;
  if (bHasLeftHit && bHasRightHit) {
    float leftDistance = FVector::Dist(
      leftHit.GetComponent()->GetComponentLocation(),
      StaticMeshRoot->GetComponentLocation()
    );
    float rightDistance = FVector::Dist(
      leftHit.GetComponent()->GetComponentLocation(),
      StaticMeshRoot->GetComponentLocation()
    );
    if (leftDistance < rightDistance) {
      newSnapTo = Cast<UBoxComponent>(leftHit.GetComponent());
      SnapIndicatorRight->SetVisibility(false);
      SnapIndicatorExternalLeft->SetVisibility(false);
    } else {
      newSnapTo = Cast<UBoxComponent>(rightHit.GetComponent());
      SnapIndicatorLeft->SetVisibility(false);
      SnapIndicatorExternalRight->SetVisibility(false);
    }
  } else if (bHasLeftHit) {
    newSnapTo = Cast<UBoxComponent>(leftHit.GetComponent());
    SnapIndicatorRight->SetVisibility(false);
    SnapIndicatorExternalLeft->SetVisibility(false);
  } else if (bHasRightHit) {
    newSnapTo = Cast<UBoxComponent>(rightHit.GetComponent());
    SnapIndicatorLeft->SetVisibility(false);
    SnapIndicatorExternalRight->SetVisibility(false);
  }
  
  if (newSnapTo != SnapToSide) {
    SnapToSide = newSnapTo;
    if (!SnapToSide) ResetMeshPosition();
  }
}

void AVCVModule::GetModulePosition(FVector& Location, FRotator& Rotation) {
  Location = StaticMeshComponent->GetComponentLocation();
  Rotation = StaticMeshComponent->GetComponentRotation();
}

void AVCVModule::GetModuleLandingPosition(FVector& Location, FRotator& Rotation, bool bOffset) {
  GetModulePosition(Location, Rotation);
  if (bOffset) Location -= StaticMeshComponent->GetForwardVector() * (2 * MODULE_DEPTH * RENDER_SCALE);
}