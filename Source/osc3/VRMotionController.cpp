#include "VRMotionController.h"

#include "osc3.h"
#include "VRAvatar.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

#include "Kismet/GameplayStatics.h"

AVRMotionController::AVRMotionController() {
	PrimaryActorTick.bCanEverTick = true;
  
  MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
  SetRootComponent(MotionController);
  
  GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GrabSphere"));
  GrabSphere->InitSphereRadius(GrabSphereRadius);
  GrabSphere->SetupAttachment(MotionController);
  
  InteractCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractCapsule"));
  InteractCapsule->SetupAttachment(MotionController);
  InteractCapsule->InitCapsuleSize(InteractCapsuleRadius, InteractCapsuleHalfHeight);
}

void AVRMotionController::BeginPlay() {
	Super::BeginPlay();
  
  // GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::LogOverlap);

  GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberBeginOverlap);
  GrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberEndOverlap);

  GrabSphere->ComponentTags.Add(TAG_GRABBER);

  InteractCapsule->SetWorldRotation(FRotator(-90.f + InteractCapsuleAngleOffset, 0.f, 0.f));
  InteractCapsule->AddLocalOffset(FVector(0.f, 0.f, InteractCapsuleHalfHeight + InteractCapsuleForwardOffset));

  InteractCapsule->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::HandleInteractorBeginOverlap);
  InteractCapsule->OnComponentEndOverlap.AddDynamic(this, &AVRMotionController::HandleInteractorEndOverlap);

  PlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

void AVRMotionController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  DrawDebugSphere(GetWorld(), GrabSphere->GetComponentLocation(), GrabSphere->GetUnscaledSphereRadius(), 16, FColor::Blue);
  DrawDebugCapsule(
    GetWorld(),
    InteractCapsule->GetComponentLocation(),
    InteractCapsule->GetUnscaledCapsuleHalfHeight(),
    InteractCapsule->GetUnscaledCapsuleRadius(),
    InteractCapsule->GetComponentRotation().Quaternion(),
    FColor::Blue
  );
}

void AVRMotionController::LogOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherCompomponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  // UE_LOG(LogTemp, Warning, TEXT("%s overlap %s"), *HandName, *OtherActor->GetActorNameOrLabel());

  if (OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s hand can grab %s"), *HandName, *OtherActor->GetActorNameOrLabel());
  }

  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) || OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s hand can interact %s"), *HandName, *OtherActor->GetActorNameOrLabel());
  }
}

void AVRMotionController::HandleInteractorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsInteracting) return;

  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) || OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    ParamActorToInteract = OtherActor;
    Cast<AVRAvatar>(GetOwner())->SetControllerParamInteracting(
      MotionController->GetTrackingSource(),
      true
    );
  }

  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM)) {
    UE_LOG(LogTemp, Warning, TEXT("%s hand can interact param %s"), *HandName, *OtherActor->GetActorNameOrLabel());
    PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
  } else if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    UE_LOG(LogTemp, Warning, TEXT("%s hand can interact port %s"), *HandName, *OtherActor->GetActorNameOrLabel());
    PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
  }
}

void AVRMotionController::StartParamInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s start param interact"), *HandName);
  bIsInteracting = true;
}

void AVRMotionController::EndParamInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s end param interact"), *HandName);
  bIsInteracting = false;

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);

  if (!overlappingActors.Contains(ParamActorToInteract)) {
    ParamActorToInteract = nullptr;
    Cast<AVRAvatar>(GetOwner())->SetControllerParamInteracting(
      MotionController->GetTrackingSource(),
      false
    );
  }
}

void AVRMotionController::HandleInteractorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing || bIsInteracting) return;

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);

  if ((OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) || OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) && !overlappingActors.Contains(ParamActorToInteract)) {
    UE_LOG(LogTemp, Display, TEXT("%s end param/port interact"), *HandName);
    Cast<AVRAvatar>(GetOwner())->SetControllerParamInteracting(
      MotionController->GetTrackingSource(),
      false
    );
  }
}

void AVRMotionController::HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsInteracting) return;

  if (OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    UE_LOG(LogTemp, Display, TEXT("%s set ActorToGrab %s"), *HandName, *OtherActor->GetActorNameOrLabel());

    ActorToGrab = OtherActor;
    Cast<AVRAvatar>(GetOwner())->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      true
    );
  }
}

void AVRMotionController::StartGrab() {
  UE_LOG(LogTemp, Display, TEXT("%s start grab"), *HandName);
  bIsGrabbing = true;
  PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
}

void AVRMotionController::EndGrab() {
  UE_LOG(LogTemp, Display, TEXT("%s end grab"), *HandName);

  bIsGrabbing = false;

  TSet<AActor*> overlappingActors;
  GrabSphere->GetOverlappingActors(overlappingActors);

  if (!overlappingActors.Contains(ActorToGrab)) {
    ActorToGrab = nullptr;
    Cast<AVRAvatar>(GetOwner())->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      false
    );
  }
}

void AVRMotionController::HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing || bIsInteracting) return;

  TSet<AActor*> overlappingActors;
  GrabSphere->GetOverlappingActors(overlappingActors);

  if (OtherActor->ActorHasTag(TAG_GRABBABLE) && !overlappingActors.Contains(ActorToGrab)) {
    UE_LOG(LogTemp, Display, TEXT("%s end overlap"), *HandName);
    ActorToGrab = nullptr;
    Cast<AVRAvatar>(GetOwner())->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      false
    );
  }
}