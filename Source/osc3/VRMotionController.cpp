#include "VRMotionController.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVModule.h"
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
  SetTooltipVisibility(false);
  TooltipWidget = Cast<UTooltip>(TooltipWidgetComponent->GetUserWidgetObject());
}

void AVRMotionController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  DrawDebugSphere(
    GetWorld(),
    GrabSphere->GetComponentLocation(),
    GrabSphere->GetUnscaledSphereRadius(),
    16,
    FColor::Blue
  );
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

void AVRMotionController::RefreshTooltip() {
  if (ParamActorToInteract) {
    FString label, displayValue;
    Cast<AVCVParam>(ParamActorToInteract)->GetTooltipText(label, displayValue);
    
    if (displayValue.IsEmpty()) {
      TooltipWidget->SetText(label);
    } else {
      TooltipWidget->SetText(label, displayValue, true);
    }
  } else if (DestinationPortActor && OriginPortActor) {
    FString fromName, _fromDescription;
    AVCVPort* fromActor = 
        OriginPortActor->getIdentity().type == PortType::Output
          ? OriginPortActor
          : DestinationPortActor;
    fromActor->GetTooltipText(fromName, _fromDescription);
    FString toName, _toDescription;
    AVCVPort* toActor = 
        OriginPortActor->getIdentity().type == PortType::Input
          ? OriginPortActor
          : DestinationPortActor;
    toActor->GetTooltipText(toName, _toDescription);

    TooltipWidget->SetText(
      FString("from: ").Append(fromName),
      FString("to: ").Append(toName)
    );
  } else if (OriginPortActor) {
    FString name, description;
    OriginPortActor->GetTooltipText(name, description);

    if (bIsPortInteracting) {
      FString prefix(
        OriginPortActor->getIdentity().type == PortType::Input
          ? "to: "
          : "from: "
      );
      TooltipWidget->SetText(prefix.Append(name));
    } else if (description.IsEmpty()) {
      TooltipWidget->SetText(name);
    } else {
      TooltipWidget->SetText(name, description, false, true);
    }
  }
}

void AVRMotionController::HandleInteractorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsParamInteracting) return;

  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) && !bIsPortInteracting) {
    ParamActorToInteract = OtherActor;
    OriginPortActor = nullptr;

    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      true
    );

    PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    
    SetTooltipVisibility(true);
    
    return;
  }
  
  if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
    if (!bIsPortInteracting) {
      Avatar->SetControllerParamOrPortInteracting(
        MotionController->GetTrackingSource(),
        true
      );

      OriginPortActor = Cast<AVCVPort>(OtherActor);
      ParamActorToInteract = nullptr;
      PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    } else if (HeldCable && Cast<AVCVPort>(OtherActor)->canConnect(HeldCable->getHangingType())) {
      DestinationPortActor = Cast<AVCVPort>(OtherActor);
      PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
    }
    
    SetTooltipVisibility(true);
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

    SetTooltipVisibility(false);
  }
}

void AVRMotionController::StartPortInteract() {
  UE_LOG(LogTemp, Display, TEXT("%s start port interact"), *HandName);
  bIsPortInteracting = true;

  int64_t cableId;
  if (OriginPortActor->getCableId(cableId)) {
    HeldCable = GameMode->DetachCable(cableId, OriginPortActor->getIdentity());
    UE_LOG(LogTemp, Warning, TEXT("got cable %lld"), cableId);
    OriginPortActor = GameMode->GetPortActor(HeldCable->getConnectedPortIdentity());
    RefreshTooltip();
  } else {
    UE_LOG(LogTemp, Warning, TEXT("spawning new cable"));
    VCVCable cable(-1);
    cable.setIdentity(OriginPortActor->getIdentity());
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

  if (!overlappingActors.Contains(OriginPortActor) && !overlappingActors.Contains(DestinationPortActor)) {
    OriginPortActor = nullptr;
    DestinationPortActor = nullptr;

    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      false
    );

    SetTooltipVisibility(false);
  } else if (overlappingActors.Contains(DestinationPortActor)) {
    OriginPortActor = DestinationPortActor;
    DestinationPortActor = nullptr;
    RefreshTooltip();
  } else {
    DestinationPortActor = nullptr;
    RefreshTooltip();
  }
}

void AVRMotionController::HandleInteractorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (bIsGrabbing || bIsParamInteracting) return;
  
  if (bIsPortInteracting) {
    if (OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT)) {
      DestinationPortActor = nullptr;
      RefreshTooltip();
    }
    return;
  }

  TSet<AActor*> overlappingActors;
  InteractCapsule->GetOverlappingActors(overlappingActors);
  
  bool wasInteracting =
    OtherActor->ActorHasTag(TAG_INTERACTABLE_PARAM) ||
    OtherActor->ActorHasTag(TAG_INTERACTABLE_PORT);
  bool stillInteracting =
    overlappingActors.Contains(ParamActorToInteract) ||
    overlappingActors.Contains(OriginPortActor) ||
    overlappingActors.Contains(DestinationPortActor);
  if (wasInteracting && !stillInteracting) {
    UE_LOG(LogTemp, Display, TEXT("%s end param/port interact"), *HandName);
    Avatar->SetControllerParamOrPortInteracting(
      MotionController->GetTrackingSource(),
      false
    );
    
    SetTooltipVisibility(false);
  }
}

void AVRMotionController::HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {
  if (bIsGrabbing || bIsParamInteracting) return;

  if (OtherActor->ActorHasTag(TAG_GRABBABLE)) {
    UE_LOG(LogTemp, Display, TEXT("%s set ActorToGrab %s"), *HandName, *OtherActor->GetActorNameOrLabel());

    ActorToGrab = OtherActor;
    Cast<AVCVModule>(ActorToGrab)->SetHighlighted(true);
    Avatar->SetControllerGrabbing(
      MotionController->GetTrackingSource(),
      true
    );
  }
}

void AVRMotionController::StartGrab() {
  UE_LOG(LogTemp, Display, TEXT("%s start grab"), *HandName);
  bIsGrabbing = true;
  SetTooltipVisibility(false);
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
  } else {
    Cast<AVCVModule>(ActorToGrab)->SetHighlighted(true);
  }
}

void AVRMotionController::HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (OtherActor->ActorHasTag(TAG_GRABBABLE))
    Cast<AVCVModule>(OtherActor)->SetHighlighted(false);
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

void AVRMotionController::SetTooltipVisibility(bool bVisible) {
  if (bVisible && !bTooltipEnabled) return;
  TooltipWidgetComponent->SetVisibility(bVisible, true);
  if (bVisible) RefreshTooltip();
}