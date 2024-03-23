#include "Utility/ModuleWeldment.h"

#include "osc3GameModeBase.h"
#include "VCVModule.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
// #include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"

AModuleWeldment::AModuleWeldment() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);
}

void AModuleWeldment::BeginPlay() {
	Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AModuleWeldment::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

  for (AVCVModule* module : Modules) module->SetWeldment(nullptr);
}

void AModuleWeldment::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  // DrawDebugSphere(
  //   GetWorld(),
  //   GetActorLocation(),
  //   5.f,
  //   16,
  //   FColor::Black
  // );
  if (SnapToSide) Snap();
}

void AModuleWeldment::SnapModeTick() {
  FHitResult rightHit = Modules[0]->RunRightSnapTrace();
  FHitResult leftHit = Modules[Modules.Num() - 1]->RunLeftSnapTrace();

  UBoxComponent* newSnapTo{nullptr};
  if (leftHit.GetActor()) {
    newSnapTo = Cast<UBoxComponent>(leftHit.GetComponent());
  } else if (rightHit.GetActor()) {
    newSnapTo = Cast<UBoxComponent>(rightHit.GetComponent());
  }

  if (newSnapTo != SnapToSide) {
    SnapToSide = newSnapTo;
    if (!SnapToSide) Unsnap();
  }
}

void AModuleWeldment::Snap(bool bTemporarily) {
  FVector offset;
  FRotator rotation;
  UBoxComponent* nextSide = SnapToSide;
  AVCVModule* moduleToSnap;

  for (int i = 0; i < Modules.Num(); i++) {
    moduleToSnap =
      SnapToSide->GetCollisionObjectType() == RIGHT_SNAP_OBJECT
        ? Modules[i]
        : Modules[Modules.Num() - (i + 1)];

    moduleToSnap->GetSnapOffset(nextSide, offset, rotation);
    if (bTemporarily) {
      moduleToSnap->OffsetMesh(offset, rotation);
    } else {
      moduleToSnap->ResetMeshPosition();
      moduleToSnap->SetActorRotation(rotation);
      moduleToSnap->AddActorWorldOffset(offset);
    }

    nextSide =
      SnapToSide->GetCollisionObjectType() == RIGHT_SNAP_OBJECT
        ? moduleToSnap->SnapColliderRight
        : moduleToSnap->SnapColliderLeft;
  }
}

void AModuleWeldment::Unsnap() {
  for (AVCVModule* module : Modules) module->ResetMeshPosition();
}

void AModuleWeldment::GetModules(TArray<AVCVModule*>& outModules) {
  for (AVCVModule* module : Modules) outModules.Push(module);
}

void AModuleWeldment::GetModuleIds(TArray<int64>& outModuleIds) {
  for (AVCVModule* module : Modules) outModuleIds.Push(module->Id);
}

void AModuleWeldment::ValidateModuleInclusion(AVCVModule* Module) {
  checkf(
    !Modules.Contains(Module),
    TEXT("module can only be in a weldment once")
  );
  checkf(
    !Module->IsInWeldment(),
    TEXT("module can only be in one weldment")
  );
}

void AModuleWeldment::AddModuleFront(AVCVModule* Module) {
  ValidateModuleInclusion(Module);
  Modules.Insert(Module, 0);
  AddModule(Module);
}

void AModuleWeldment::AddModuleBack(AVCVModule* Module) {
  ValidateModuleInclusion(Module);
  Modules.Push(Module);
  AddModule(Module);
}

void AModuleWeldment::AddModule(AVCVModule* Module) {
  Module->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
  Module->SetWeldment(this);
  ResetLocation();
  HighlightModules();
}

void AModuleWeldment::Append(AModuleWeldment* OtherWeldment) {
  TArray<AVCVModule*> otherModules;
  OtherWeldment->GetModules(otherModules);
  GameMode->DestroyWeldment(OtherWeldment);

  for (AVCVModule* module : otherModules) {
    AddModuleBack(module);
  }
}

void AModuleWeldment::ResetLocation() {
  FVector averageLocation{0.f};

  for (AVCVModule* module : Modules)
    averageLocation = averageLocation + module->GetActorLocation();
  averageLocation = averageLocation / (float)Modules.Num();

  FVector offset = averageLocation - GetActorLocation();

  AddActorWorldOffset(offset);
  for (AVCVModule* module : Modules)
    module->AddActorWorldOffset(-offset);
}

void AModuleWeldment::EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  GrabOffset = GrabbedLocation - GetActorLocation();

  for (AVCVModule* module : Modules)
    module->AddActorWorldOffset(-GrabOffset);
  SetActorLocation(GrabbedLocation);

  LastGrabbedRotation = GrabbedRotation;
  LastGrabbedLocation = GrabbedLocation;
  LastLocationDelta = FVector(0.f, 0.f, 0.f);
}

void AModuleWeldment::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  // return;
  FQuat qFrom = LastGrabbedRotation.Quaternion();
  // TODO: user
  FQuat qTo = FQuat::Slerp(qFrom, GrabbedRotation.Quaternion(), GRABBABLE_ROTATION_SMOOTHING_FACTOR_DEFAULT);
  FQuat qDelta = qTo * qFrom.Inverse();
  
  FVector locationDelta = GrabbedLocation - LastGrabbedLocation;
  locationDelta = UKismetMathLibrary::WeightedMovingAverage_FVector(
    locationDelta,
    LastLocationDelta,
    // TODO: user
    GRABBABLE_LOCATION_SMOOTHING_FACTOR_DEFAULT
  );
  LastLocationDelta = locationDelta;

  AddActorWorldOffset(locationDelta);
  AddActorWorldRotation(qDelta);

  LastGrabbedLocation = GrabbedLocation;
  LastGrabbedRotation = qTo.Rotator();
  
  for (AVCVModule* module : Modules)
    module->TriggerCableUpdates();
}

void AModuleWeldment::ReleaseGrab() {
  if (SnapToSide) {
    Unsnap();
    Snap(false);

    AVCVModule* leader =
      SnapToSide->GetCollisionObjectType() == RIGHT_SNAP_OBJECT
        ? Modules[0]
        : Modules[Modules.Num() - 1];
    AVCVModule* snapToModule = Cast<AVCVModule>(SnapToSide->GetOwner());

    if (SnapToSide->GetCollisionObjectType() == RIGHT_SNAP_OBJECT) {
      GameMode->WeldModules(snapToModule, leader);
    } else {
      GameMode->WeldModules(leader, snapToModule);
    }

    SnapToSide = nullptr;
  }
}

void AModuleWeldment::HighlightModules() {
  for (AVCVModule* module : Modules) module->SetHighlighted(true);

  FTimerHandle hUnhighlight;
  GetWorld()->GetTimerManager().SetTimer(
    hUnhighlight,
    this,
    &AModuleWeldment::UnhighlightModules,
    0.4f, // seconds
    false // loop
  );
}

void AModuleWeldment::UnhighlightModules() {
  for (AVCVModule* module : Modules) module->SetHighlighted(false);
}