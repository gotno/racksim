#include "Utility/ModuleWeldment.h"

#include "osc3GameModeBase.h"
#include "VCVModule.h"

#include "Algo/Reverse.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
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
  //   2.f,
  //   16,
  //   FColor::Black
  // );
}

void AModuleWeldment::GetModules(TArray<AVCVModule*>& outModules) {
  for (AVCVModule* module : Modules) outModules.Push(module);
}

void AModuleWeldment::GetModuleIds(TArray<int64>& outModuleIds) {
  for (AVCVModule* module : Modules) outModuleIds.Push(module->Id);
}

bool AModuleWeldment::Contains(AActor* Actor) {
  return Modules.Contains(Actor);
}

int32 AModuleWeldment::IndexOf(AActor* Actor) {
  if (!Contains(Actor)) return -1;
  return Modules.IndexOfByPredicate([&](AVCVModule* module) { return module == Actor; });
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

void AModuleWeldment::PositionModule(int32 ModuleIndex, int32 RelativeToIndex) {
  AVCVModule* moduleToPosition = Modules[ModuleIndex];
  AVCVModule* positionRelativeTo = Modules[RelativeToIndex];
  FSnapModeSide alignToSide =
    ModuleIndex < RelativeToIndex
      ? FSnapModeSide::Left
      : FSnapModeSide::Right;

  moduleToPosition->AlignActorTo(positionRelativeTo, alignToSide);
}

void AModuleWeldment::AddModuleFront(AVCVModule* Module) {
  ValidateModuleInclusion(Module);
  Modules.Insert(Module, 0);
  if (Modules.Num() > 1) PositionModule(0, 1);
  AddModule(Module);
}

void AModuleWeldment::AddModuleBack(AVCVModule* Module) {
  ValidateModuleInclusion(Module);
  Modules.Push(Module);
  if (Modules.Num() > 1) PositionModule(Modules.Num() - 1, Modules.Num() - 2);
  AddModule(Module);
}

void AModuleWeldment::AddModule(AVCVModule* Module) {
  Module->SetWeldment(this);
  Module->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
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

void AModuleWeldment::Prepend(AModuleWeldment* OtherWeldment) {
  TArray<AVCVModule*> otherModules;
  OtherWeldment->GetModules(otherModules);
  Algo::Reverse(otherModules);
  GameMode->DestroyWeldment(OtherWeldment);

  for (AVCVModule* module : otherModules) {
    AddModuleFront(module);
  }
}

bool AModuleWeldment::SplitIfAdjacent(AVCVModule* leftModule, AVCVModule* rightModule) {
  int splitIndex{-1};
  bool bAdjacent{false};

  for (int i = 0; i < Modules.Num() - 1; i++) {
    splitIndex = i;
    bAdjacent =
      (Modules[i] == leftModule && Modules[i + 1] == rightModule)
        || (Modules[i] == rightModule && Modules[i + 1] == leftModule);

    if (bAdjacent) break;
  }

  if (bAdjacent) GameMode->SplitWeldment(this, splitIndex);
  return bAdjacent;
}

void AModuleWeldment::Reset() {
  ResetPositions();
  ResetLocation();
}

void AModuleWeldment::ResetPositions() {
  if (Modules.Num() < 2) return;
  for (int i = 1; i < Modules.Num(); i++) PositionModule(i, i - 1);
}

void AModuleWeldment::ResetLocation() {
  FVector averageLocation{0.f};

  for (AVCVModule* module : Modules)
    averageLocation += module->GetActorLocation();
  averageLocation /= (float)Modules.Num();

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

void AModuleWeldment::InitSnapMode() {
  Modules[0]->SetSnapMode(FSnapModeSide::Left);
  Modules[Modules.Num() - 1]->SetSnapMode(FSnapModeSide::Right);
}

void AModuleWeldment::CancelSnapMode() {
  Modules[0]->SetSnapMode(FSnapModeSide::None);
  Modules[Modules.Num() - 1]->SetSnapMode(FSnapModeSide::None);
}

bool AModuleWeldment::IsInSnapMode() {
  return Modules[0]->IsInSnapMode(true)
    || Modules[Modules.Num() - 1]->IsInSnapMode(true);
}

void AModuleWeldment::FollowSnap(AVCVModule* Module) {
  if (Module == nullptr) {
    for (AVCVModule* module : Modules) module->ResetMeshPosition();
    InitSnapMode();
    return;
  }

  if (Module == Modules[0]) {
    Modules[Modules.Num() - 1]->SetSnapMode(FSnapModeSide::None);
    for (int i = 1; i <= Modules.Num() - 1; i++) {
      Modules[i]->AlignMeshTo(Modules[i - 1], FSnapModeSide::Right);
    }
  }

  if (Module == Modules[Modules.Num() - 1]) {
    Modules[0]->SetSnapMode(FSnapModeSide::None);
    for (int i = Modules.Num() - 2; i >= 0; i--) {
      Modules[i]->AlignMeshTo(Modules[i + 1], FSnapModeSide::Left);
    }
  }
}