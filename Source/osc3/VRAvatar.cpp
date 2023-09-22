#include "VRAvatar.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "osc3.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputTriggers.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVRAvatar::AVRAvatar() {
	PrimaryActorTick.bCanEverTick = true;
  
  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(VRRoot);
  
  DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination"));
  DestinationMarker->SetupAttachment(VRRoot);
  DestinationMarker->SetVisibility(false);

  LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
  LeftController->SetupAttachment(VRRoot);
  LeftController->SetTrackingSource(EControllerHand::Left);

  RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
  RightController->SetupAttachment(VRRoot);
  RightController->SetTrackingSource(EControllerHand::Right);
}

void AVRAvatar::BeginPlay() {
	Super::BeginPlay();
  DestinationMarker->SetVisibility(false);
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
      InputSubsystem->AddMappingContext(InputMappingContexts.WorldManipulation, 0);
    }
  }	
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

  // teleport
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTeleport);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTeleport);
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTeleport, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTeleport, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TeleportLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTeleport);
  Input->BindAction(WorldManipulationActions.TeleportRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTeleport);
  
  // rotate world
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.RotateWorldLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotateWorld);
  Input->BindAction(WorldManipulationActions.RotateWorldRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotateWorld);

  // translate world
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTranslateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTranslateWorld, EControllerHand::Left);
  Input->BindAction(WorldManipulationActions.TranslateWorldLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTranslateWorld);

  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartTranslateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleTranslateWorld, EControllerHand::Right);
  Input->BindAction(WorldManipulationActions.TranslateWorldRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteTranslateWorld);

  // roto-translate world
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Started, this, &AVRAvatar::HandleStartRotoTranslateWorld);
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Triggered, this, &AVRAvatar::HandleRotoTranslateWorld);
  Input->BindAction(WorldManipulationActions.RotoTranslateWorld, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteRotoTranslateWorld);

  // quit
  Input->BindAction(WorldManipulationActions.Quit, ETriggerEvent::Completed, this, &AVRAvatar::Quit);
}

void AVRAvatar::LogInput(const FInputActionValue& _Value, FString msg) {
  UE_LOG(LogTemp, Warning, TEXT("input: %s"), *msg);
}

// teleport
void AVRAvatar::HandleStartTeleport(const FInputActionValue& _Value) {
  LastDestinationLocation = FVector::ZeroVector;
}
void AVRAvatar::HandleTeleport(const FInputActionValue& Value, EControllerHand hand) {
  if (Value.GetMagnitude() > 0.f) {
    SweepDestination(hand);
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
void AVRAvatar::HandleCompleteTeleport(const FInputActionValue& _Value) {
  if (HasDestinationHit) {
    FVector offset = DestinationHitResult.Location - Camera->GetComponentLocation();
    offset.Z = 0.f;
    AddActorWorldOffset(offset);
  }
  HasDestinationHit = false;
  DestinationMarker->SetVisibility(false);
}
void AVRAvatar::SweepDestination(EControllerHand hand) {
  UMotionControllerComponent* controller =
    hand == EControllerHand::Left ? LeftController : RightController;
  HasDestinationHit =
    GetWorld()->LineTraceSingleByChannel(
      DestinationHitResult,
      controller->GetComponentLocation(),
      controller->GetComponentLocation() + controller->GetForwardVector() * 1000,
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
void AVRAvatar::HandleStartRotateWorld(const FInputActionValue& _Value, EControllerHand hand) {
  if (hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetComponentLocation();
  } else {
    LastRightHandLocation = RightController->GetComponentLocation();
  }
}
void AVRAvatar::HandleRotateWorld(const FInputActionValue& _Value, EControllerHand hand) {
  FVector controllerLocation;
  FVector lastControllerLocation;

  if (hand == EControllerHand::Left) {
    controllerLocation = LeftController->GetComponentLocation();
    lastControllerLocation = LastLeftHandLocation;
  } else {
    controllerLocation = RightController->GetComponentLocation();
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

  if (hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetComponentLocation();
  } else {
    LastRightHandLocation = RightController->GetComponentLocation();
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

void AVRAvatar::HandleCompleteRotateWorld(const FInputActionValue& _Value) {
  LastRotateWorldDelta = 0.f;
}

// translate world
void AVRAvatar::HandleStartTranslateWorld(const FInputActionValue& _Value, EControllerHand hand) {
  FString handLabel(hand == EControllerHand::Left ? "left" : "right");

  if (hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetComponentLocation();
  } else {
    LastRightHandLocation = RightController->GetComponentLocation();
  }
}
void AVRAvatar::HandleCompleteTranslateWorld(const FInputActionValue& _Value) {
  LastTranslateWorldDelta = FVector::ZeroVector;
}
void AVRAvatar::HandleTranslateWorld(const FInputActionValue& _Value, EControllerHand hand) {
  FVector cameraLocation = Camera->GetComponentLocation();
  FVector controllerLocation, lastControllerLocation;

  if (hand == EControllerHand::Left) {
    controllerLocation = LeftController->GetComponentLocation();
    lastControllerLocation = LastLeftHandLocation;
  } else {
    controllerLocation = RightController->GetComponentLocation();
    lastControllerLocation = LastRightHandLocation;
  }

  FVector locationDelta = (lastControllerLocation - cameraLocation) - (controllerLocation - cameraLocation); 
  locationDelta.Z = 0.f;
  locationDelta *= TranslateWorldScale;

  locationDelta =
    UKismetMathLibrary::WeightedMovingAverage_FVector(locationDelta, LastTranslateWorldDelta, 0.2f);
  LastTranslateWorldDelta = locationDelta;

  AddActorWorldOffset(locationDelta);

  if (hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetComponentLocation();
  } else {
    LastRightHandLocation = RightController->GetComponentLocation();
  }
}

void AVRAvatar::HandleStartRotoTranslateWorld(const FInputActionValue& _Value) {
  LastLeftHandLocation = LeftController->GetComponentLocation();
  LastRightHandLocation = RightController->GetComponentLocation();
}
void AVRAvatar::HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value) {
  LastTranslateWorldDelta = FVector::ZeroVector;
  LastRotateWorldDelta = 0.f;
}
void AVRAvatar::HandleRotoTranslateWorld(const FInputActionValue& _Value) {
  FVector leftControllerLocation = LeftController->GetComponentLocation();
  FVector rightControllerLocation = RightController->GetComponentLocation();

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

  LastLeftHandLocation = LeftController->GetComponentLocation();
  LastRightHandLocation = RightController->GetComponentLocation();
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
  location = GetActorLocation() + Camera->GetForwardVector() * 10.f;
  rotation = Camera->GetForwardVector().Rotation();
}