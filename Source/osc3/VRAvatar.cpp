#include "VRAvatar.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "osc3.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "InputTriggers.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"
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
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
      Subsystem->ClearAllMappings();
      Subsystem->AddMappingContext(InputMappingContext, 0);
    }
  }	
}

float AVRAvatar::GetRotationalDistanceBetweenControllerPositions(const FVector& c1, const FVector& c2) {
  FVector rootLocation = Camera->GetComponentLocation();
  rootLocation.Z = 0.f;

  FVector c1Vector = FVector(c1.X, c1.Y, 0.f) - rootLocation;
  c1Vector.Normalize();
  FVector c2Vector = FVector(c2.X, c2.Y, 0.f) - rootLocation;
  c2Vector.Normalize();
  float distance =
    FMath::RadiansToDegrees(
      FMath::Acos(FVector::DotProduct(c2Vector, c1Vector))
    );
  
  return c1Vector.Rotation().Yaw < c2Vector.Rotation().Yaw ? -distance : distance;
}

void AVRAvatar::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  // adjust for offset from playspace center
  // FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
  // NewCameraOffset.Z = 0.f;
  // AddActorWorldOffset(NewCameraOffset);
  // VRRoot->AddWorldOffset(-NewCameraOffset);
  
  FVector vrRootLocation = VRRoot->GetComponentLocation();
  vrRootLocation.Z = 10.f;
  FVector cameraLocation = Camera->GetComponentLocation();
  cameraLocation.Z = 10.f;
  DrawDebugSphere(GetWorld(), vrRootLocation, 5.f, 32, FColor::Red);
  DrawDebugSphere(GetWorld(), cameraLocation, 10.f, 32, FColor::Blue);
  DrawDebugSphere(GetWorld(), GetActorLocation(), 5.f, 32, FColor::Green);
}

void AVRAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
  
  UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

  // teleport
  // Input->BindAction(InputActions.Teleport, ETriggerEvent::Started, this, &AVRAvatar::StartTeleport);
  // Input->BindAction(InputActions.Teleport, ETriggerEvent::Triggered, this, &AVRAvatar::SweepDestination);
  // Input->BindAction(InputActions.Teleport, ETriggerEvent::Completed, this, &AVRAvatar::CompleteTeleport);
  
  // left/right grip
  Input->BindAction(InputActions.RightGrip, ETriggerEvent::Started, this, &AVRAvatar::StartRightGrip);
  Input->BindAction(InputActions.RightGrip, ETriggerEvent::Completed, this, &AVRAvatar::CompleteRightGrip);
  Input->BindAction(InputActions.LeftGrip, ETriggerEvent::Started, this, &AVRAvatar::StartLeftGrip);
  Input->BindAction(InputActions.LeftGrip, ETriggerEvent::Completed, this, &AVRAvatar::CompleteLeftGrip);

  // rotate world
  Input->BindAction(InputActions.RotateWorld, ETriggerEvent::Started, this, &AVRAvatar::StartRotateWorld);
  Input->BindAction(InputActions.RotateWorld, ETriggerEvent::Triggered, this, &AVRAvatar::RotateWorld);
  Input->BindAction(InputActions.RotateWorld, ETriggerEvent::Completed, this, &AVRAvatar::CompleteRotateWorld);

  // translate world
  // Input->BindAction(InputActions.TranslateWorld, ETriggerEvent::Started, this, &AVRAvatar::StartTranslateWorld);
  // Input->BindAction(InputActions.TranslateWorld, ETriggerEvent::Triggered, this, &AVRAvatar::TranslateWorld);
  // Input->BindAction(InputActions.TranslateWorld, ETriggerEvent::Completed, this, &AVRAvatar::CompleteTranslateWorld);
}

// teleport
void AVRAvatar::StartTeleport(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("start teleport"));
  bTeleportActive = true;
  LastDestinationLocation = FVector::ZeroVector;
}
void AVRAvatar::CompleteTeleport(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete teleport"));
  bTeleportActive = false;
  if (HasDestinationHit) {
    FVector currentLocation = GetActorLocation();
    FVector destinationLocation = DestinationHitResult.Location;
    SetActorLocation(FVector(destinationLocation.X, destinationLocation.Y, destinationLocation.Z));
    DestinationMarker->SetVisibility(false);
  }
}
void AVRAvatar::SweepDestination(const FInputActionValue& _Value) {
  if (bWorldTranslateActive) return;

  // const FName TraceTag("Destination Trace Tag");
  // GetWorld()->DebugDrawTraceTag = TraceTag;
  FCollisionQueryParams CollisionParams;
  // CollisionParams.TraceTag = TraceTag;

  UMotionControllerComponent* controller = bRightGripActive ? RightController : LeftController;
  HasDestinationHit =
    GetWorld()->LineTraceSingleByChannel(
      DestinationHitResult,
      controller->GetComponentLocation(),
      controller->GetComponentLocation() + controller->GetForwardVector() * 1000,
      TELEPORT_TRACE,
      CollisionParams
    );

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

// grips
void AVRAvatar::StartRightGrip(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("start right grip"));
  bRightGripActive = true;
  LastRightHandLocation = RightController->GetComponentLocation();
}
void AVRAvatar::CompleteRightGrip(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete right grip"));
  bRightGripActive = false;
}

void AVRAvatar::StartLeftGrip(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("start left grip"));
  bLeftGripActive = true;
  LastLeftHandLocation = LeftController->GetComponentLocation();
}
void AVRAvatar::CompleteLeftGrip(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete left grip"));
  bLeftGripActive = false;
}

// rotate world
void AVRAvatar::StartRotateWorld(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("start rotate world"));
  LastRotateWorldDegrees = 0.f;
}
void AVRAvatar::CompleteRotateWorld(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete rotate world"));
}
void AVRAvatar::RotateWorld(const FInputActionValue& _Value) {
  // if (bWorldTranslateActive || bTeleportActive) return;

  FVector leftControllerLocation = LeftController->GetComponentLocation();
  FVector rightControllerLocation = RightController->GetComponentLocation();

  // TODO: favor dominant hand?
  float rotateWorldDegrees =
    GetRotationalDistanceBetweenControllerPositions(
      bLeftGripActive ? LastLeftHandLocation : LastRightHandLocation,
      bLeftGripActive ? leftControllerLocation : rightControllerLocation
    ) * RotateWorldScale;
  
  rotateWorldDegrees =
    FMath::WeightedMovingAverage(rotateWorldDegrees, LastRotateWorldDegrees, 0.2f);
  LastRotateWorldDegrees = rotateWorldDegrees;

  // RotateWorldByDegrees(rotateWorldDegrees);
  // d10j

  FRotator inputRotation(0.f, rotateWorldDegrees, 0.f);

  // UE_LOG(LogTemp, Warning, TEXT("rotating %fdeg"), rotateWorldDegrees);

  FVector vrOrigin = VRRoot->GetComponentLocation();
  FVector cameraLocation = Camera->GetComponentLocation();
  FVector translation = vrOrigin - cameraLocation;
  FVector rotated = translation.RotateAngleAxis(rotateWorldDegrees, FVector::UpVector);
  VRRoot->SetWorldLocation(cameraLocation + rotated);
  VRRoot->AddRelativeRotation(inputRotation);

  LastLeftHandLocation = LeftController->GetComponentLocation();
  LastRightHandLocation = RightController->GetComponentLocation();
}

// translate world
void AVRAvatar::StartTranslateWorld(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("start translate"));
  bWorldTranslateActive = true;
  LastTranslateWorldOffset = FVector::ZeroVector;
  LastLeftHandLocation = LeftController->GetComponentLocation();
  LastRightHandLocation = RightController->GetComponentLocation();
}
void AVRAvatar::CompleteTranslateWorld(const FInputActionValue& _Value) {
  UE_LOG(LogTemp, Warning, TEXT("complete translate"));
  bWorldTranslateActive = false;
}
void AVRAvatar::TranslateWorld(const FInputActionValue& _Value) {
  FVector currentActorLocation = GetActorLocation();
  FVector leftControllerLocation = LeftController->GetComponentLocation();
  FVector rightControllerLocation = RightController->GetComponentLocation();
  FVector rootLocation = VRRoot->GetComponentLocation();

  FVector leftChange = (LastLeftHandLocation - rootLocation) - (leftControllerLocation - rootLocation);
  FVector rightChange = (LastRightHandLocation - rootLocation) - (rightControllerLocation - rootLocation);

  float xChange =
    FMath::Abs(leftChange.X) > FMath::Abs(rightChange.X) ? leftChange.X : rightChange.X;
  float yChange =
    FMath::Abs(leftChange.Y) > FMath::Abs(rightChange.Y) ? leftChange.Y : rightChange.Y;
  
  FVector translateOffset =
    FVector(
      xChange * TranslateWorldScale,
      yChange * TranslateWorldScale,
      0.f
    );
  translateOffset =
    UKismetMathLibrary::WeightedMovingAverage_FVector(translateOffset, LastTranslateWorldOffset, 0.2f);
  LastTranslateWorldOffset = translateOffset;

  AddActorWorldOffset(translateOffset);

  LastLeftHandLocation = LeftController->GetComponentLocation();
  LastRightHandLocation = RightController->GetComponentLocation();
}

void AVRAvatar::MoveForward(float throttle) {
  AddMovementInput(Camera->GetForwardVector(), throttle);
}

void AVRAvatar::MoveRight(float throttle) {
  AddMovementInput(Camera->GetRightVector(), throttle);
}

// get the position+rotation for spawning a WidgetSurrogate in the camera's view
// so it is invisible (facing away) but renders immediately
void AVRAvatar::GetRenderablePosition(FVector& location, FRotator& rotation) {
  location = GetActorLocation() + Camera->GetForwardVector() * 10.f;
  rotation = Camera->GetForwardVector().Rotation();
}