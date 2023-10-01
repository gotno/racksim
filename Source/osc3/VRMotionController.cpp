#include "VRMotionController.h"

#include "osc3.h"
#include "VCVModule.h"
#include "VCVParam.h"
#include "VCVPort.h"

#include "VRAvatar.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

AVRMotionController::AVRMotionController() {
	PrimaryActorTick.bCanEverTick = true;
  
  MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
  SetRootComponent(MotionController);
  
  GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GrabSphere"));
  GrabSphere->InitSphereRadius(2.f);
  GrabSphere->SetupAttachment(MotionController);
}

void AVRMotionController::BeginPlay() {
	Super::BeginPlay();
  
  // GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::LogOverlap);

  GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberBeginOverlap);
  GrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberEndOverlap);

  GrabSphere->ComponentTags.Add(TAG_GRABBER);
}

void AVRMotionController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  DrawDebugSphere(GetWorld(), GrabSphere->GetComponentLocation(), GrabSphere->GetUnscaledSphereRadius(), 16, FColor::Blue);
}

void AVRMotionController::LogOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherCompomponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s hand can grab %s"), *HandName, *OtherActor->GetActorNameOrLabel());
  }
}

void AVRMotionController::HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (!bIsGrabbing && OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    UE_LOG(LogTemp, Display, TEXT("%s set ActorToGrab %s"), *HandName, *OtherActor->GetActorNameOrLabel());

    ActorToGrab = OtherActor;
    Cast<AVRAvatar>(GetOwner())->SetControllerInteracting(
      MotionController->GetTrackingSource(),
      true
    );
  }
}

void AVRMotionController::StartGrab() {
  UE_LOG(LogTemp, Display, TEXT("%s start grab"), *HandName);
  bIsGrabbing = true;
}

void AVRMotionController::EndGrab() {
  UE_LOG(LogTemp, Display, TEXT("%s end grab"), *HandName);
  bIsGrabbing = false;
  ActorToGrab = nullptr;
  Cast<AVRAvatar>(GetOwner())->SetControllerInteracting(
    MotionController->GetTrackingSource(),
    false
  );
}

void AVRMotionController::HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing) return;

  TSet<UPrimitiveComponent*> overlappingComponents;
  GrabSphere->GetOverlappingComponents(overlappingComponents);

  if (OtherActor->ActorHasTag(TAG_GRABBABLE) && overlappingComponents.IsEmpty()) {
    UE_LOG(LogTemp, Display, TEXT("%s end overlap"), *HandName);
    Cast<AVRAvatar>(GetOwner())->SetControllerInteracting(
      MotionController->GetTrackingSource(),
      false
    );
  }
}

AActor* AVRMotionController::GetActorToGrab() {
  return ActorToGrab;
}