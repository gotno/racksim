#include "VRAvatar.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "osc3.h"

#include "osc3GameModeBase.h"
#include "VCVModule.h"
#include "VCVParam.h"
#include "VCVSlider.h"
#include "VCVKnob.h"
#include "VCVPort.h"
#include "VCVCable.h"

#include "VRMotionController.h"
#include "MotionControllerComponent.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputTriggers.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVRAvatar::AVRAvatar() {
	PrimaryActorTick.bCanEverTick = true;
  
  Cast<UCapsuleComponent>(GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(VRRoot);
  
  DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination"));
  DestinationMarker->SetupAttachment(VRRoot);
  DestinationMarker->SetVisibility(false);
}

void AVRAvatar::BeginPlay() {
	Super::BeginPlay();
  DestinationMarker->SetVisibility(false);
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVRAvatar::PawnClientRestart() {
  Super::PawnClientRestart();

  UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

  // fix for character height being offset from root capsule component
  float halfHeight = Cast<UCapsuleComponent>(GetRootComponent())->GetScaledCapsuleHalfHeight();
  VRRoot->SetRelativeLocation(FVector(0, 0, -halfHeight));
  Camera->SetRelativeLocation(FVector(0, 0, -halfHeight));

  if (APlayerController* PC = Cast<APlayerController>(GetController())) {
    InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (InputSubsystem) {
      InputSubsystem->ClearAllMappings();
      InputSubsystem->AddMappingContext(InputMappingContexts.Base, 0);
      InputSubsystem->AddMappingContext(InputMappingContexts.WorldManipulationLeft, 1);
      InputSubsystem->AddMappingContext(InputMappingContexts.WorldManipulationRight, 1);
    }
  }	
  
  LeftController = GetWorld()->SpawnActor<AVRMotionController>(MotionControllerClass);
  LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
  LeftController->SetOwner(this);
  LeftController->SetTrackingSource(EControllerHand::Left);

  RightController = GetWorld()->SpawnActor<AVRMotionController>(MotionControllerClass);
  RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
  RightController->SetOwner(this);
  RightController->SetTrackingSource(EControllerHand::Right);
}

void AVRAvatar::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  FVector vrRootLocation = VRRoot->GetComponentLocation();
  vrRootLocation.Z = 10.f;
  FVector cameraLocation = Camera->GetComponentLocation();
  cameraLocation.Z = 10.f;
  DrawDebugSphere(GetWorld(), vrRootLocation, 5.f, 32, FColor::Red);
  DrawDebugSphere(GetWorld(), cameraLocation, 10.f, 32, FColor::Blue);
  DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Green);

  DrawDebugSphere(GetWorld(), FVector(200.f, 200.f, 10.f), 5.f, 32, FColor::Magenta);
  DrawDebugSphere(GetWorld(), FVector(-200.f, 200.f, 10.f), 5.f, 32, FColor::Magenta);
  DrawDebugSphere(GetWorld(), FVector(-200.f, -200.f, 10.f), 5.f, 32, FColor::Magenta);
  DrawDebugSphere(GetWorld(), FVector(200.f, -200.f, 10.f), 5.f, 32, FColor::Magenta);
}

void AVRAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
  
  UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

  // world manipulation
  // teleport
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTeleport, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTeleport, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTeleport, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTeleport, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTeleport, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTeleport, EControllerHand::Right);
  
  // rotate world
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotateWorld, EControllerHand::Right);

  // translate world
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTranslateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTranslateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTranslateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTranslateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTranslateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTranslateWorld, EControllerHand::Right);

  // roto-translate world
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotoTranslateWorld);
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotoTranslateWorld);
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotoTranslateWorld);

  // module manipulation
  // grab
  Input->BindAction(ModuleManipulationActions.GrabLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartGrab, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.GrabRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartGrab, EControllerHand::Right);
  Input->BindAction(ModuleManipulationActions.GrabLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleGrab, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.GrabRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleGrab, EControllerHand::Right);
  Input->BindAction(ModuleManipulationActions.GrabLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteGrab, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.GrabRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteGrab, EControllerHand::Right);

  // duplicate module
  Input->BindAction(ModuleManipulationActions.DuplicateModuleLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleDuplicateModule, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.DuplicateModuleRight, ETriggerEvent::Started, this, &AVRAvatar::HandleDuplicateModule, EControllerHand::Right);

  // param interaction
  // engage
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartParamEngage, EControllerHand::Right);
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleParamEngage, EControllerHand::Right);
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteParamEngage, EControllerHand::Right);

  // general
  // quit
  Input->BindAction(BaseActions.Quit, ETriggerEvent::Completed, this, &AVRAvatar::Quit);
}

void AVRAvatar::SetControllerGrabbing(EControllerHand Hand, bool bGrabbing) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* moduleManipulationMappingContext = 
    Hand == EControllerHand::Left
      ? InputMappingContexts.ModuleManipulationLeft
      : InputMappingContexts.ModuleManipulationRight;

  UE_LOG(LogTemp, Warning, TEXT("setting module grabbing mapping %d"), bGrabbing);

  if (bGrabbing) {
    InputSubsystem->AddMappingContext(moduleManipulationMappingContext, 2);
  } else {
    InputSubsystem->RemoveMappingContext(moduleManipulationMappingContext);
  }
}

void AVRAvatar::SetControllerParamOrPortInteracting(EControllerHand Hand, bool bInteracting) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* paramInteractingMappingContext = 
    Hand == EControllerHand::Left
      ? InputMappingContexts.ParamInteractionLeft
      : InputMappingContexts.ParamInteractionRight;
  
  UE_LOG(LogTemp, Warning, TEXT("setting param interacting mapping %d"), bInteracting);

  if (bInteracting) {
    InputSubsystem->AddMappingContext(paramInteractingMappingContext, 3);
  } else {
    InputSubsystem->RemoveMappingContext(paramInteractingMappingContext);
  }
}

void AVRAvatar::LogInput(const FInputActionValue& _Value, FString msg) {
  UE_LOG(LogTemp, Warning, TEXT("input: %s"), *msg);
}


void AVRAvatar::SetWorldManipulationActive(EControllerHand Hand, bool Active) {
  if (Hand == EControllerHand::Left) {
    bLeftHandWorldManipulationActive = Active;
  } else {
    bRightHandWorldManipulationActive = Active;
  }
}

// teleport
void AVRAvatar::HandleStartTeleport(const FInputActionValue& _Value, EControllerHand Hand) {
  LastDestinationLocation = FVector::ZeroVector;
  SetWorldManipulationActive(Hand, true);
}
void AVRAvatar::HandleTeleport(const FInputActionValue& Value, EControllerHand Hand) {
  if (Value.GetMagnitude() > 0.f) {
    SweepDestination(Hand);
  }

  if (HasDestinationHit) {
    FVector destinationLocation =
      UKismetMathLibrary::WeightedMovingAverage_FVector(
        DestinationHitResult.Location,
        LastDestinationLocation, 0.2f
      );
    LastDestinationLocation = destinationLocation;
    DestinationMarker->SetWorldLocation(destinationLocation);
    DestinationMarker->SetVisibility(true);
  } else {
    DestinationMarker->SetVisibility(false);
  }
}
void AVRAvatar::HandleCompleteTeleport(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, false);
  if (HasDestinationHit) {
    FVector offset = DestinationHitResult.Location - Camera->GetComponentLocation();
    offset.Z = 0.f;
    AddActorWorldOffset(offset);
  }
  HasDestinationHit = false;
  DestinationMarker->SetVisibility(false);
}
void AVRAvatar::SweepDestination(EControllerHand Hand) {
  AVRMotionController* controller =
    Hand == EControllerHand::Left ? LeftController : RightController;
  FVector controllerLocation = controller->GetActorLocation();
  HasDestinationHit =
    GetWorld()->LineTraceSingleByChannel(
      DestinationHitResult,
      controllerLocation,
      controllerLocation + controller->GetActorForwardVector() * 1000,
      TELEPORT_TRACE
    );
}

float AVRAvatar::GetRotationalDistanceBetweenControllerPositions(const FVector& c1, const FVector& c2) {
  FVector rootLocation = Camera->GetComponentLocation();
  rootLocation.Z = 0.f;
  FVector c1Vector = FVector(c1.X, c1.Y, 0.f) - rootLocation;
  c1Vector.Normalize();
  FVector c2Vector = FVector(c2.X, c2.Y, 0.f) - rootLocation;

  return c1Vector.Rotation().Yaw - c2Vector.Rotation().Yaw;
}

// rotate world
void AVRAvatar::HandleStartRotateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, true);
  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}
void AVRAvatar::HandleRotateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  FVector controllerLocation;
  FVector lastControllerLocation;

  if (Hand == EControllerHand::Left) {
    controllerLocation = LeftController->GetActorLocation();
    lastControllerLocation = LastLeftHandLocation;
  } else {
    controllerLocation = RightController->GetActorLocation();
    lastControllerLocation = LastRightHandLocation;
  }


  float deltaYaw =
    GetRotationalDistanceBetweenControllerPositions(
      lastControllerLocation,
      controllerLocation
    ) * RotateWorldScale;
  
  deltaYaw =
    FMath::WeightedMovingAverage(deltaYaw, LastRotateWorldDelta, 0.2f);
  LastRotateWorldDelta = deltaYaw;
  
  RotateWorldAroundPivot(deltaYaw, Camera->GetComponentLocation());

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}
void AVRAvatar::RotateWorldAroundPivot(float degrees, FVector pivot) {
  DrawDebugSphere(GetWorld(), pivot, 2.f, 12, FColor::Emerald);
  FVector vrOrigin = VRRoot->GetComponentLocation();
  FVector translation = vrOrigin - pivot;
  FVector rotatedTranslation = translation.RotateAngleAxis(degrees, FVector::UpVector);
  VRRoot->SetWorldLocation(pivot + rotatedTranslation);
  VRRoot->AddRelativeRotation(FRotator (0.f, degrees, 0.f));
}

void AVRAvatar::HandleCompleteRotateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, false);
  LastRotateWorldDelta = 0.f;
}

// translate world
void AVRAvatar::HandleStartTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, true);
  FString handLabel(Hand == EControllerHand::Left ? "left" : "right");

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}
void AVRAvatar::HandleCompleteTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, false);
  LastTranslateWorldDelta = FVector::ZeroVector;
}
void AVRAvatar::HandleTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand) {
  FVector cameraLocation = Camera->GetComponentLocation();
  FVector controllerLocation, lastControllerLocation;

  if (Hand == EControllerHand::Left) {
    controllerLocation = LeftController->GetActorLocation();
    lastControllerLocation = LastLeftHandLocation;
  } else {
    controllerLocation = RightController->GetActorLocation();
    lastControllerLocation = LastRightHandLocation;
  }

  FVector locationDelta = (lastControllerLocation - cameraLocation) - (controllerLocation - cameraLocation); 
  locationDelta.Z = 0.f;
  locationDelta *= TranslateWorldScale;

  locationDelta =
    UKismetMathLibrary::WeightedMovingAverage_FVector(locationDelta, LastTranslateWorldDelta, 0.2f);
  LastTranslateWorldDelta = locationDelta;

  AddActorWorldOffset(locationDelta);

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}

void AVRAvatar::HandleStartRotoTranslateWorld(const FInputActionValue& _Value) {
  // bLeftHandRotoTranslateActive = true;

  UE_LOG(LogTemp, Warning, TEXT("start roto-translate"));
  SetWorldManipulationActive(EControllerHand::Left, true);
  SetWorldManipulationActive(EControllerHand::Right, true);
  LastLeftHandLocation = LeftController->GetActorLocation();
  LastRightHandLocation = RightController->GetActorLocation();
}
void AVRAvatar::HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete roto-translate"));
  SetWorldManipulationActive(EControllerHand::Left, false);
  SetWorldManipulationActive(EControllerHand::Right, false);
  LastTranslateWorldDelta = FVector::ZeroVector;
  LastRotateWorldDelta = 0.f;
}
void AVRAvatar::HandleRotoTranslateWorld(const FInputActionValue& _Value) {
  FVector leftControllerLocation = LeftController->GetActorLocation();
  FVector rightControllerLocation = RightController->GetActorLocation();

  FRotator lastRotation = (LastLeftHandLocation - LastRightHandLocation).Rotation();
  FRotator thisRotation = (leftControllerLocation - rightControllerLocation).Rotation();
  float deltaYaw = lastRotation.Yaw - thisRotation.Yaw;

  deltaYaw =
    FMath::WeightedMovingAverage(deltaYaw, LastRotateWorldDelta, 0.2f);
  LastRotateWorldDelta = deltaYaw;
  
  FVector pivot = (leftControllerLocation - rightControllerLocation) / 2 + rightControllerLocation;
  
  RotateWorldAroundPivot(deltaYaw, pivot);

  FVector cameraLocation = Camera->GetComponentLocation();
  FVector leftLocationDelta = (LastLeftHandLocation - cameraLocation) - (leftControllerLocation - cameraLocation); 
  leftLocationDelta.Z = 0.f;
  FVector rightLocationDelta = (LastRightHandLocation - cameraLocation) - (rightControllerLocation - cameraLocation); 
  rightLocationDelta.Z = 0.f;

  FVector locationDelta = (leftLocationDelta + rightLocationDelta) / 2;

  locationDelta =
    UKismetMathLibrary::WeightedMovingAverage_FVector(locationDelta, LastTranslateWorldDelta, 0.2f);
  LastTranslateWorldDelta = locationDelta;

  AddActorWorldOffset(locationDelta);

  // UE_LOG(LogTemp, Warning, TEXT("ran roto-translate %f %s"), deltaYaw, *locationDelta.ToCompactString());

  LastLeftHandLocation = LeftController->GetActorLocation();
  LastRightHandLocation = RightController->GetActorLocation();
}

void AVRAvatar::HandleStartGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  AVCVModule* grabbedModule = Cast<AVCVModule>(controller->GetActorToGrab());

  UE_LOG(LogTemp, Warning, TEXT("%s hand grab start"), *FString(Hand == EControllerHand::Left ? "left" : "right"));
  if (grabbedModule) {
    UE_LOG(LogTemp, Warning, TEXT("  grabbing %s"), *grabbedModule->GetActorNameOrLabel());
    controller->StartGrab();
    grabbedModule->EngageGrab(controller->GetActorLocation(), controller->GetActorRotation());
  }
}

void AVRAvatar::HandleGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;

  AVCVModule* grabbedModule = Cast<AVCVModule>(controller->GetActorToGrab());

  if (grabbedModule) {
    grabbedModule->AlterGrab(controller->GetActorLocation(), controller->GetActorRotation());
  }
}

void AVRAvatar::HandleCompleteGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  AVCVModule* grabbedModule = Cast<AVCVModule>(controller->GetActorToGrab());

  UE_LOG(LogTemp, Warning, TEXT("%s hand grab complete"), *FString(Hand == EControllerHand::Left ? "left" : "right"));
  if (grabbedModule) {
    UE_LOG(LogTemp, Warning, TEXT("  releasing %s"), *grabbedModule->GetActorNameOrLabel());
    grabbedModule->ReleaseGrab();
    controller->EndGrab();
  }
}

void AVRAvatar::HandleDuplicateModule(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  AVCVModule* grabbedModule = Cast<AVCVModule>(controller->GetActorToGrab());

  if (grabbedModule) {
    UE_LOG(LogTemp, Warning, TEXT("%s duplicate module %s"), Hand == EControllerHand::Left ? *FString("left") : *FString("right"), *grabbedModule->GetActorNameOrLabel());
    GameMode->DuplicateModule(grabbedModule);
  }
}

void AVRAvatar::HandleStartParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  UE_LOG(LogTemp, Warning, TEXT("%s hand param engage start"), *FString(Hand == EControllerHand::Left ? "left" : "right"));

  AVCVParam* interactingParam = Cast<AVCVParam>(controller->GetParamActorToInteract());
  if (interactingParam) {
    UE_LOG(LogTemp, Warning, TEXT("  engaging param %s"), *interactingParam->GetActorNameOrLabel());
    controller->StartParamInteract();

    if (Cast<AVCVKnob>(interactingParam)) {
      interactingParam->engage(controller->GetActorRotation().Roll);
    } else if (Cast<AVCVSlider>(interactingParam)) {
      interactingParam->engage(controller->GetActorLocation());
    } else {
      interactingParam->engage();
    }
  }

  AVCVPort* interactingPort = controller->GetPortActorToInteract();
  if (interactingPort) {
    controller->StartPortInteract();
  }
}

void AVRAvatar::HandleParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  AVCVParam* interactingParam = Cast<AVCVParam>(controller->GetParamActorToInteract());
  if (interactingParam) {
    if (Cast<AVCVKnob>(interactingParam)) {
      interactingParam->alter(controller->GetActorRotation().Roll);
    } else if (Cast<AVCVSlider>(interactingParam)) {
      interactingParam->alter(controller->GetActorLocation());
    }
    controller->RefreshTooltip();
  }

  AVCVCable* heldCable = controller->GetHeldCable();
  if (heldCable) {
    AVCVPort* destinationPort = controller->GetDestinationPortActor();
    if (destinationPort) {
      FVector location, direction;
      GameMode->GetPortInfo(destinationPort->getIdentity(), location, direction);
      heldCable->setHangingLocation(location, direction);
    } else {
      heldCable->setHangingLocation(
        controller->GetActorLocation() + controller->GetActorForwardVector() * 5.f,
        -controller->GetActorForwardVector()
      );
    }
  }
}

void AVRAvatar::HandleCompleteParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = 
    Hand == EControllerHand::Left ? LeftController : RightController;
  
  UE_LOG(LogTemp, Warning, TEXT("%s hand param engage complete"), *FString(Hand == EControllerHand::Left ? "left" : "right"));

  AVCVParam* interactingParam = Cast<AVCVParam>(controller->GetParamActorToInteract());
  if (interactingParam) {
    UE_LOG(LogTemp, Warning, TEXT("  releasing %s"), *interactingParam->GetActorNameOrLabel());

    controller->EndParamInteract();
    interactingParam->release();
  }

  AVCVCable* heldCable = controller->GetHeldCable();
  if (heldCable) {
    controller->EndPortInteract();
  }
}

void AVRAvatar::Quit(const FInputActionValue& _Value) {
  UKismetSystemLibrary::QuitGame(
    GetWorld(),
    UGameplayStatics::GetPlayerController(this, 0),
    EQuitPreference::Quit,
    false
  );
}

// get the position+rotation for spawning a WidgetSurrogate in the camera's view
// so it is invisible (facing away) but renders immediately
void AVRAvatar::GetRenderablePosition(FVector& location, FRotator& rotation) {
  // location = Camera->GetComponentLocation() + Camera->GetForwardVector() * 10.f;
  // rotation = Camera->GetForwardVector().Rotation();
  location = GetActorLocation() + GetActorForwardVector() * 10.f;
  rotation = GetActorForwardVector().Rotation();
}

FRotator AVRAvatar::GetLookAtCameraRotation(FVector FromPosition) {
  return UKismetMathLibrary::FindLookAtRotation(
    FromPosition,
    Camera->GetComponentLocation()
  );
}