#include "VRMotionController.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVParam.h"
#include "VCVCable.h"
#include "VCVPort.h"
#include "VRAvatar.h"
#include "Tooltip.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
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

  TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Tooltip"));
  TooltipWidgetComponent->SetupAttachment(MotionController);
}

void AVRMotionController::BeginPlay() {
	Super::BeginPlay();
  
  Avatar = Cast<AVRAvatar>(GetOwner());
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
  // GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::LogOverlap);

  GrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberBeginOverlap);
  GrabSphere->OnComponentEndOverlap.AddDynamic(this, &AVRMotionController::HandleGrabberEndOverlap);

  GrabSphere->AddLocalOffset(MotionController->GetUpVector() * GrabSphereUpOffset);
  GrabSphere->ComponentTags.Add(TAG_GRABBER);

  InteractCapsule->SetWorldRotation(FRotator(-90.f + InteractCapsuleAngleOffset, 0.f, 0.f));
  InteractCapsule->AddLocalOffset(FVector(0.f, 0.f, InteractCapsuleHalfHeight + InteractCapsuleForwardOffset));
  InteractCapsule->SetCollisionObjectType(INTERACTOR_OBJECT);

  InteractCapsule->OnComponentBeginOverlap.AddDynamic(this, &AVRMotionController::HandleInteractorBeginOverlap);
  InteractCapsule->OnComponentEndOverlap.AddDynamic(this, &AVRMotionController::HandleInteractorEndOverlap);

  PlayerController = UGameplayStatics::GetPlayerController(this, 0);

  TooltipWidgetComponent->SetWorldScale3D(FVector(0.04f, 0.04f, 0.04f));
  TooltipWidgetComponent->SetVisibility(false, true);
  TooltipWidget = Cast<UTooltip>(TooltipWidgetComponent->GetUserWidgetObject());
  if (TooltipWidget) {
    TooltipWidget->SetLabel(FString("brabel"));
    TooltipWidget->SetDisplayValue(FString("display bralue"));
  }
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

  // Tooltip->SetWorldLocation(MotionController->GetComponentLocation() + MotionController->GetUpVector() * 5.f);
  TooltipWidgetComponent->SetWorldRotation(
    Avatar->GetLookAtCameraRotation(TooltipWidgetComponent->GetComponentLocation())
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

void AVRMotionController::UpdateTooltip() {
  FString label, displayValue;
  Cast<AVCVParam>(ParamActorToInteract)->GetTooltipText(label, displayValue);
  TooltipWidget->SetLabel(label);
  TooltipWidget->SetDisplayValue(displayValue);
}

void AVRMotionController::HandleInteractorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsParamInteracting) return;

  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) && !bIsPortInteracting) {
    ParamActorToInteract = OtherActor;
    PortActorToInteract = nullptr;

    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      true
    );

    PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    
    TooltipWidgetComponent->SetVisibility(true, true);
    UpdateTooltip();
    
    return;
  }
  
  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    if (!bIsPortInteracting) {
      Avatar->SetControllerParamOrPortInteracting(
        MotionController->GetTrackingSource(),
        true
      );

      PortActorToInteract = Cast<AVCVPort>(OtherActor);
      ParamActorToInteract = nullptr;
      PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    } else if (HeldCable && Cast<AVCVPort>(OtherActor)->canConnect(HeldCable->getHangingType())) {
      DestinationPortActor = Cast<AVCVPort>(OtherActor);
      PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    }
  }
}

void AVRMotionController::StartParamInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s start param interact"), *HandName);
  bIsParamInteracting = true;
}

void AVRMotionController::EndParamInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s end param interact"), *HandName);
  bIsParamInteracting = false;

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);

  if (!overlappingActors.Contains(ParamActorToInteract)) {
    ParamActorToInteract = nullptr;
    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      false
    );

    TooltipWidgetComponent->SetVisibility(false, true);
  }
}

void AVRMotionController::StartPortInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s start port interact"), *HandName);
  bIsPortInteracting = true;

  int64_t cableId;
  if (PortActorToInteract->getCableId(cableId)) {
    HeldCable = GameMode->DetachCable(cableId, PortActorToInteract->getIdentity());
    UE_LOG(LogTemp, Warning, TEXT("got cable %lld"), cableId);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("spawning new cable"));
    VCVCable cable(-1);
    cable.setIdentity(PortActorToInteract->getIdentity());
    HeldCable = GameMode->SpawnCable(cable);
  }
}

void AVRMotionController::EndPortInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s end port interact"), *HandName);

  if (DestinationPortActor && DestinationPortActor->canConnect(HeldCable->getHangingType())) {
    GameMode->AttachCable(HeldCable->getId(), DestinationPortActor->getIdentity());
  } else {
    GameMode->DestroyCable(HeldCable->getId());
  }

  bIsPortInteracting = false;
  HeldCable = nullptr;

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);

  if (!overlappingActors.Contains(PortActorToInteract) && !overlappingActors.Contains(DestinationPortActor)) {
    PortActorToInteract = nullptr;
    DestinationPortActor = nullptr;

    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      false
    );
  } else if (overlappingActors.Contains(DestinationPortActor)) {
    PortActorToInteract = DestinationPortActor;
    DestinationPortActor = nullptr;
  } else {
    DestinationPortActor = nullptr;
  }
}

void AVRMotionController::HandleInteractorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing || bIsParamInteracting) return;
  
  if (bIsPortInteracting && OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    DestinationPortActor = nullptr;
    return;
  } else if (bIsPortInteracting) {
    return;
  }

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);

  if ((OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) || OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) && !overlappingActors.Contains(ParamActorToInteract)) {
    UE_LOG(LogTemp, Display, TEXT("%s end param/port interact"), *HandName);
    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      false
    );
    
    TooltipWidgetComponent->SetVisibility(false, true);
  }
}

void AVRMotionController::HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsParamInteracting) return;

  if (OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    UE_LOG(LogTemp, Display, TEXT("%s set ActorToGrab %s"), *HandName, *OtherActor->GetActorNameOrLabel());

    ActorToGrab = OtherActor;
    Avatar->SetControllerGrabbing(
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
    Avatar->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      false
    );
  }
}

void AVRMotionController::HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing || bIsParamInteracting) return;

  TSet<AActor*> overlappingActors;
  GrabSphere->GetOverlappingActors(overlappingActors);

  if (OtherActor->ActorHasTag(TAG_GRABBABLE) && !overlappingActors.Contains(ActorToGrab)) {
    UE_LOG(LogTemp, Display, TEXT("%s end overlap"), *HandName);
    ActorToGrab = nullptr;
    Avatar->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      false
    );
  }
}