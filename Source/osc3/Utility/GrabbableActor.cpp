#include "Utility/GrabbableActor.h"

#include "osc3GameModeBase.h"
#include "Utility/ModuleWeldment.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AGrabbableActor::AGrabbableActor() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);

  StaticMeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Static Mesh Root"));
  StaticMeshRoot->SetupAttachment(GetRootComponent());

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetupAttachment(StaticMeshRoot);
  StaticMeshComponent->SetRenderInDepthPass(true);
  StaticMeshComponent->SetRenderCustomDepth(true);
  StaticMeshComponent->SetCustomDepthStencilValue(2);

  OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Outline Mesh"));
  OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  OutlineMeshComponent->SetupAttachment(StaticMeshComponent);
  OutlineMeshComponent->SetVisibility(false);

  static ConstructorHelpers::FObjectFinder<UMaterial> OutlineMaterial(TEXT("/Script/Engine.Material'/Game/materials/looman_outlines/M_LocalOutlines.M_LocalOutlines'"));
  if (OutlineMaterial.Object) OutlineMaterialInterface = Cast<UMaterial>(OutlineMaterial.Object);
}

void AGrabbableActor::BeginPlay() {
	Super::BeginPlay();

  if (OutlineMaterialInterface) {
    OutlineMaterialInstance = UMaterialInstanceDynamic::Create(OutlineMaterialInterface, this);
    OutlineMeshComponent->SetMaterial(0, OutlineMaterialInstance);
  }

  Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this))->SubscribeGrabbableSetDelegate(this);
}

void AGrabbableActor::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AGrabbableActor::SetHighlighted(bool bHighlighted, FLinearColor OutlineColor) {
  OutlineMeshComponent->SetVisibility(bHighlighted);
  OutlineMaterialInstance->SetVectorParameterValue(FName("Color"), OutlineColor);
}

void AGrabbableActor::EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab engage"), *GetActorNameOrLabel());
  bGrabEngaged = true;
  if (!IsInWeldment()) SetHighlighted(false);

  if (Weldment) {
    Weldment->EngageGrab(GrabbedLocation, GrabbedRotation);
    return;
  }

  GrabOffset = GrabbedLocation - GetActorLocation();

  StaticMeshRoot->AddWorldOffset(-GrabOffset);
  SetActorLocation(GrabbedLocation);

  LastGrabbedRotation = GrabbedRotation;
  LastGrabbedLocation = GrabbedLocation;
  LastLocationDelta = FVector(0.f, 0.f, 0.f);
}

void AGrabbableActor::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  if (Weldment) {
    Weldment->AlterGrab(GrabbedLocation, GrabbedRotation);
    return;
  }

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
}

void AGrabbableActor::ReleaseGrab() {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab release"), *GetActorNameOrLabel());
  bGrabEngaged = false;
  SetHighlighted(true);

  FVector staticMeshLocation = StaticMeshRoot->GetComponentLocation();
  FVector finalOffset = GetActorLocation() - staticMeshLocation;
  SetActorLocation(staticMeshLocation);
  StaticMeshRoot->AddWorldOffset(finalOffset);

  GrabOffset = FVector(0.f);
}

void AGrabbableActor::HighlightIfTargeted(AActor* GrabbableTarget, EControllerHand Hand) {
  if (GrabbableTarget == this) {
    if (Hand == EControllerHand::Left) bTargetGrabOfLeftHand = true;
    if (Hand == EControllerHand::Right) bTargetGrabOfRightHand = true;
  } else {
    if (Hand == EControllerHand::Left) bTargetGrabOfLeftHand = false;
    if (Hand == EControllerHand::Right) bTargetGrabOfRightHand = false;
  }

  SetHighlighted(bTargetGrabOfLeftHand || bTargetGrabOfRightHand);
}