#include "Player/VRAvatar.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "osc3.h"

#include "osc3GameModeBase.h"
#include "osc3GameState.h"
#include "VCVModule.h"
#include "Utility/ModuleWeldment.h"
#include "ModuleComponents/VCVParam.h"
#include "ModuleComponents/VCVSlider.h"
#include "ModuleComponents/VCVKnob.h"
#include "ModuleComponents/VCVPort.h"
#include "VCVCable.h"
#include "CableEnd.h"
#include "Library.h"
#include "Utility/GrabbableActor.h"

#include "Player/VRMotionController.h"
#include "MotionControllerComponent.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "InputTriggers.h"
#include "EnhancedInputSubsystems.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVRAvatar::AVRAvatar() {
	PrimaryActorTick.bCanEverTick = true;
  
  Cast<UCapsuleComponent>(GetRootComponent())->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
  VRRoot->SetupAttachment(GetRootComponent());

  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
  Camera->SetupAttachment(VRRoot);
}

void AVRAvatar::BeginPlay() {
	Super::BeginPlay();
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  GameState = Cast<Aosc3GameState>(UGameplayStatics::GetGameState(this));
}

void AVRAvatar::PawnClientRestart() {
  Super::PawnClientRestart();

  UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

  // fix for character height being offset from root capsule component
  float halfHeight = Cast<UCapsuleComponent>(GetRootComponent())->GetScaledCapsuleHalfHeight() + 12.f;
  VRRoot->SetRelativeLocation(FVector(0, 0, -halfHeight));
  Camera->SetRelativeLocation(FVector(0, 0, -halfHeight));

  if (APlayerController* PC = Cast<APlayerController>(GetController())) {
    InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (InputSubsystem) {
      InputSubsystem->ClearAllMappings();
      InputSubsystem->AddMappingContext(InputMappingContexts.Base, 0);
    }
  }

  if (!DestinationMarker) {
    DestinationMarker = GetWorld()->SpawnActor<AActor>(DestinationMarkerClass);
    DestinationMarker->SetOwner(this);
    DestinationMarker->SetActorHiddenInGame(true);
  }

  if (!LeftController) {
    LeftController = GetWorld()->SpawnActor<AVRMotionController>(MotionControllerClass);
    LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    LeftController->SetOwner(this);
    LeftController->SetTrackingSource(EControllerHand::Left);

    LeftController->OnGrabbableTargetedDelegate.AddUObject(this, &AVRAvatar::HandleGrabbableTargetSet);
    LeftController->OnParamTargetedDelegate.AddUObject(this, &AVRAvatar::HandleParamTargetSet);
    LeftController->OnOriginPortTargetedDelegate.AddUObject(this, &AVRAvatar::HandleOriginPortOrCableTargetSet);
    LeftController->OnCableTargetedDelegate.AddUObject(this, &AVRAvatar::HandleOriginPortOrCableTargetSet);
    LeftController->OnCableHeldDelegate.AddUObject(this, &AVRAvatar::HandleHeldCableSet);
  }

  if (!RightController) {
    RightController = GetWorld()->SpawnActor<AVRMotionController>(MotionControllerClass);
    RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
    RightController->SetOwner(this);
    RightController->SetTrackingSource(EControllerHand::Right);

    RightController->OnGrabbableTargetedDelegate.AddUObject(this, &AVRAvatar::HandleGrabbableTargetSet);
    RightController->OnParamTargetedDelegate.AddUObject(this, &AVRAvatar::HandleParamTargetSet);
    RightController->OnOriginPortTargetedDelegate.AddUObject(this, &AVRAvatar::HandleOriginPortOrCableTargetSet);
    RightController->OnCableTargetedDelegate.AddUObject(this, &AVRAvatar::HandleOriginPortOrCableTargetSet);
    RightController->OnCableHeldDelegate.AddUObject(this, &AVRAvatar::HandleHeldCableSet);
  }
}

void AVRAvatar::GetSavegamePlayerPosition(FVector& Location, FRotator& Rotation) {
  Location = VRRoot->GetComponentLocation();

  Rotation = VRRoot->GetComponentRotation();
  Rotation.Pitch = 0.f;
  Rotation.Roll = 0.f;
}

void AVRAvatar::SetPlayerPosition(FVector& Location, FRotator& Rotation) {
  VRRoot->SetWorldLocation(Location);
  VRRoot->SetWorldRotation(Rotation);
}

void AVRAvatar::EnableWorldManipulation() {
  InputSubsystem->AddMappingContext(InputMappingContexts.WorldManipulationLeft, 1);
  InputSubsystem->AddMappingContext(InputMappingContexts.WorldManipulationRight, 1);
}

void AVRAvatar::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  // DrawDebugSphere(
  //   GetWorld(),
  //   GetActorLocation(),
  //   1.f,
  //   16,
  //   FColor::Blue
  // );
  // DrawDebugSphere(
  //   GetWorld(),
  //   VRRoot->GetComponentLocation(),
  //   3.f,
  //   16,
  //   FColor::Emerald
  // );
  // FVector cameraLocation = Camera->GetComponentLocation();
  // cameraLocation.Z = 0.f;
  // DrawDebugSphere(
  //   GetWorld(),
  //   cameraLocation,
  //   1.f,
  //   16,
  //   FColor::Yellow
  // );
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

  // destroy module
  Input->BindAction(ModuleManipulationActions.DestroyModuleLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleDestroyModule, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.DestroyModuleRight, ETriggerEvent::Started, this, &AVRAvatar::HandleDestroyModule, EControllerHand::Right);

  // module manipulation, trigger: toggle context menu OR set snap mode
  Input->BindAction(ModuleManipulationActions.ModuleContextMenuOrSnapModeLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleSnapModeTriggered, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.ModuleContextMenuOrSnapModeRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleSnapModeTriggered, EControllerHand::Right);
  Input->BindAction(ModuleManipulationActions.ModuleContextMenuOrSnapModeLeft, ETriggerEvent::Canceled, this, &AVRAvatar::HandleContextMenuTriggeredOrSnapModeCancelled, EControllerHand::Left);
  Input->BindAction(ModuleManipulationActions.ModuleContextMenuOrSnapModeRight, ETriggerEvent::Canceled, this, &AVRAvatar::HandleContextMenuTriggeredOrSnapModeCancelled, EControllerHand::Right);

  // param interaction
  // engage
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartParamEngage, EControllerHand::Right);
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleParamEngage, EControllerHand::Right);
  Input->BindAction(ParamInteractionActions.ParamEngageLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteParamEngage, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamEngageRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteParamEngage, EControllerHand::Right);

  // reset
  Input->BindAction(ParamInteractionActions.ParamResetLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleParamReset, EControllerHand::Left);
  Input->BindAction(ParamInteractionActions.ParamResetRight, ETriggerEvent::Started, this, &AVRAvatar::HandleParamReset, EControllerHand::Right);

  // port/cable interaction
  // engage
  Input->BindAction(PortInteractionActions.PortEngageLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartPortEngage, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.PortEngageRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartPortEngage, EControllerHand::Right);
  Input->BindAction(PortInteractionActions.PortEngageLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandlePortEngage, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.PortEngageRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandlePortEngage, EControllerHand::Right);
  Input->BindAction(PortInteractionActions.PortEngageLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompletePortEngage, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.PortEngageRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompletePortEngage, EControllerHand::Right);
  // destroy cable
  Input->BindAction(PortInteractionActions.CableDestroyLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleCableDestroy, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.CableDestroyRight, ETriggerEvent::Started, this, &AVRAvatar::HandleCableDestroy, EControllerHand::Right);
  // toggle latched (allow cable to exist unconnected)
  Input->BindAction(PortInteractionActions.CableLatchLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleCableLatch, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.CableLatchRight, ETriggerEvent::Started, this, &AVRAvatar::HandleCableLatch, EControllerHand::Right);
  // cycle cable colors
  Input->BindAction(PortInteractionActions.CableColorCycleLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleCableColorCycle, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.CableColorCycleRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleCableColorCycle, EControllerHand::Right);
  Input->BindAction(PortInteractionActions.CableColorCycleLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteCableColorCycle, EControllerHand::Left);
  Input->BindAction(PortInteractionActions.CableColorCycleRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteCableColorCycle, EControllerHand::Right);

  // general
  // toggle main menu
  Input->BindAction(BaseActions.ToggleMenu, ETriggerEvent::Completed, this, &AVRAvatar::ToggleMainMenu);
  // summon library
  Input->BindAction(BaseActions.SummonLibraryLeft, ETriggerEvent::Completed, this, &AVRAvatar::SummonLibrary, EControllerHand::Left);
  Input->BindAction(BaseActions.SummonLibraryRight, ETriggerEvent::Completed, this, &AVRAvatar::SummonLibrary, EControllerHand::Right);
  // request screenshot
  Input->BindAction(BaseActions.RequestScreenshot, ETriggerEvent::Completed, this, &AVRAvatar::RequestScreenshot);
  // quit
  Input->BindAction(BaseActions.Quit, ETriggerEvent::Completed, this, &AVRAvatar::Quit);

  // widget interaction
  // widget left click
  Input->BindAction(WidgetManipulationActions.WidgetLeftClickLeft, ETriggerEvent::Started, this, &AVRAvatar::HandleStartWidgetLeftClick, EControllerHand::Left);
  Input->BindAction(WidgetManipulationActions.WidgetLeftClickRight, ETriggerEvent::Started, this, &AVRAvatar::HandleStartWidgetLeftClick, EControllerHand::Right);
  Input->BindAction(WidgetManipulationActions.WidgetLeftClickLeft, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteWidgetLeftClick, EControllerHand::Left);
  Input->BindAction(WidgetManipulationActions.WidgetLeftClickRight, ETriggerEvent::Completed, this, &AVRAvatar::HandleCompleteWidgetLeftClick, EControllerHand::Right);
  // widget scroll
  Input->BindAction(WidgetManipulationActions.WidgetScrollLeft, ETriggerEvent::Triggered, this, &AVRAvatar::HandleWidgetScroll, EControllerHand::Left);
  Input->BindAction(WidgetManipulationActions.WidgetScrollRight, ETriggerEvent::Triggered, this, &AVRAvatar::HandleWidgetScroll, EControllerHand::Right);
}

void AVRAvatar::SetControllerWidgetInteracting(EControllerHand Hand, bool bEnable) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* widgetManipulationMappingContext = 
    Hand == EControllerHand::Left
      ? InputMappingContexts.WidgetManipulationLeft
      : InputMappingContexts.WidgetManipulationRight;

  // UE_LOG(LogTemp, Warning, TEXT("setting widget interacting mapping %d"), bEnable);

  if (bEnable && !InputSubsystem->HasMappingContext(widgetManipulationMappingContext)) {
    InputSubsystem->AddMappingContext(widgetManipulationMappingContext, 2);
  } else {
    InputSubsystem->RemoveMappingContext(widgetManipulationMappingContext);
  }
}

void AVRAvatar::SetControllerGrabbing(EControllerHand Hand, bool bEnable) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* moduleManipulationMappingContext = 
    Hand == EControllerHand::Left
    ? InputMappingContexts.ModuleManipulationLeft
      : InputMappingContexts.ModuleManipulationRight;

  // UE_LOG(LogTemp, Warning, TEXT("setting module grabbing mapping %d"), bEnable);

  if (bEnable && !InputSubsystem->HasMappingContext(moduleManipulationMappingContext)) {
    InputSubsystem->AddMappingContext(moduleManipulationMappingContext, 3);
  } else if (!bEnable) {
    InputSubsystem->RemoveMappingContext(moduleManipulationMappingContext);
  }
}

void AVRAvatar::SetControllerParamInteracting(EControllerHand Hand, bool bEnable) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* paramInteractingMappingContext = 
    Hand == EControllerHand::Left
      ? InputMappingContexts.ParamInteractionLeft
      : InputMappingContexts.ParamInteractionRight;

  if (bEnable && !InputSubsystem->HasMappingContext(paramInteractingMappingContext)) {
    InputSubsystem->AddMappingContext(paramInteractingMappingContext, 4);
  } else if (!bEnable) {
    InputSubsystem->RemoveMappingContext(paramInteractingMappingContext);
  }
}

void AVRAvatar::SetControllerPortInteracting(EControllerHand Hand, bool bEnable) {
  if (Hand == EControllerHand::Left && bLeftHandWorldManipulationActive) return;
  if (Hand == EControllerHand::Right && bRightHandWorldManipulationActive) return;

  UInputMappingContext* portInteractingMappingContext = 
    Hand == EControllerHand::Left
      ? InputMappingContexts.PortInteractionLeft
      : InputMappingContexts.PortInteractionRight;

  if (bEnable && !InputSubsystem->HasMappingContext(portInteractingMappingContext)) {
    InputSubsystem->AddMappingContext(portInteractingMappingContext, 4);
  } else if (!bEnable) {
    InputSubsystem->RemoveMappingContext(portInteractingMappingContext);
  }
}

void AVRAvatar::LogInput(const FInputActionValue& _Value, FString msg) {
  UE_LOG(LogTemp, Warning, TEXT("input: %s"), *msg);
}

void AVRAvatar::SetWorldManipulationActive(EControllerHand Hand, bool bActive) {
  if (Hand == EControllerHand::Left) {
    bLeftHandWorldManipulationActive = bActive;
  } else {
    bRightHandWorldManipulationActive = bActive;
  }
  GetControllerForHand(Hand)->SetWorldInteract(bActive);
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

    DestinationMarker->SetActorLocation(destinationLocation, true);
    DestinationMarker->SetActorHiddenInGame(false);
  } else {
    DestinationMarker->SetActorHiddenInGame(true);
  }
}
void AVRAvatar::HandleCompleteTeleport(const FInputActionValue& _Value, EControllerHand Hand) {
  SetWorldManipulationActive(Hand, false);
  if (HasDestinationHit) {
    FVector offset = DestinationMarker->GetActorLocation() - Camera->GetComponentLocation();
    offset.Z = 0.f;
    AddActorWorldOffset(offset);
    GameState->SetUnsaved();
  }
  HasDestinationHit = false;
  DestinationMarker->SetActorHiddenInGame(true);

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}
void AVRAvatar::SweepDestination(EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
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
    FMath::WeightedMovingAverage(deltaYaw, LastRotateWorldDelta, WORLD_ROTATION_SMOOTHING_FACTOR_DEFAULT);
  LastRotateWorldDelta = deltaYaw;
  
  RotateWorldAroundPivot(deltaYaw, Camera->GetComponentLocation());
  GameState->SetUnsaved();

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}
void AVRAvatar::RotateWorldAroundPivot(float degrees, FVector pivot) {
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
    UKismetMathLibrary::WeightedMovingAverage_FVector(locationDelta, LastTranslateWorldDelta, WORLD_TRANSLATION_SMOOTHING_FACTOR_DEFAULT);
  LastTranslateWorldDelta = locationDelta;

  AddActorWorldOffset(locationDelta);
  GameState->SetUnsaved();

  if (Hand == EControllerHand::Left) {
    LastLeftHandLocation = LeftController->GetActorLocation();
  } else {
    LastRightHandLocation = RightController->GetActorLocation();
  }
}

void AVRAvatar::HandleStartRotoTranslateWorld(const FInputActionValue& _Value) {
  // UE_LOG(LogTemp, Warning, TEXT("start roto-translate"));
  SetWorldManipulationActive(EControllerHand::Left, true);
  SetWorldManipulationActive(EControllerHand::Right, true);
  LastLeftHandLocation = LeftController->GetActorLocation();
  LastRightHandLocation = RightController->GetActorLocation();
}
void AVRAvatar::HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value) {
  // UE_LOG(LogTemp, Warning, TEXT("complete roto-translate"));
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
    FMath::WeightedMovingAverage(deltaYaw, LastRotateWorldDelta, WORLD_ROTATION_SMOOTHING_FACTOR_DEFAULT);
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
    UKismetMathLibrary::WeightedMovingAverage_FVector(locationDelta, LastTranslateWorldDelta, WORLD_TRANSLATION_SMOOTHING_FACTOR_DEFAULT);
  LastTranslateWorldDelta = locationDelta;

  AddActorWorldOffset(locationDelta);
  GameState->SetUnsaved();

  // UE_LOG(LogTemp, Warning, TEXT("ran roto-translate %f %s"), deltaYaw, *locationDelta.ToCompactString());

  LastLeftHandLocation = LeftController->GetActorLocation();
  LastRightHandLocation = RightController->GetActorLocation();
}

void AVRAvatar::HandleParamTargetSet(AActor* ParamActor, EControllerHand Hand) {
  if (ParamActor) {
    SetControllerParamInteracting(Hand, true);
    if (Hand == EControllerHand::Left) {
      LeftHandParamActor = Cast<AVCVParam>(ParamActor);
    } else {
      RightHandParamActor = Cast<AVCVParam>(ParamActor);
    }
  } else {
    SetControllerParamInteracting(Hand, false);
    if (Hand == EControllerHand::Left) {
      LeftHandParamActor = nullptr;
    } else {
      RightHandParamActor = nullptr;
    }
  }
}

void AVRAvatar::HandleOriginPortOrCableTargetSet(AActor* PortOrCableActor, EControllerHand Hand) {
  if (PortOrCableActor) {
    SetControllerPortInteracting(Hand, true);
  } else {
    SetControllerPortInteracting(Hand, false);
  }
}

void AVRAvatar::HandleHeldCableSet(AActor* CableEnd, EControllerHand Hand) {
  if (CableEnd) {
    if (Hand == EControllerHand::Left) {
      LeftHandHeldCableEnd = Cast<ACableEnd>(CableEnd);
    } else {
      RightHandHeldCableEnd = Cast<ACableEnd>(CableEnd);
    }
  } else {
    if (Hand == EControllerHand::Left) {
      LeftHandHeldCableEnd = nullptr;
    } else {
      RightHandHeldCableEnd = nullptr;
    }
    SetControllerPortInteracting(Hand, false);
  }
}

void AVRAvatar::HandleGrabbableTargetSet(AActor* GrabbableActor, EControllerHand Hand) {
  if (GrabbableActor) {
    SetControllerGrabbing(Hand, true);
    if (Hand == EControllerHand::Left) {
      LeftHandGrabbableActor = Cast<AGrabbableActor>(GrabbableActor);
    } else {
      RightHandGrabbableActor = Cast<AGrabbableActor>(GrabbableActor);
    }
  } else {
    SetControllerGrabbing(Hand, false);
    if (Hand == EControllerHand::Left) {
      LeftHandGrabbableActor = nullptr;
    } else {
      RightHandGrabbableActor = nullptr;
    }
  }
}

void AVRAvatar::MaybeSplitWeldment(EControllerHand AlreadyGrabbingHand) {
  AVCVModule* leftModule = Cast<AVCVModule>(LeftHandGrabbableActor);
  AVCVModule* rightModule = Cast<AVCVModule>(RightHandGrabbableActor);

  if (!leftModule || !rightModule) return;
  if (!leftModule->IsInWeldment() || !rightModule->IsInWeldment()) return;
  if (leftModule->GetWeldment() != rightModule->GetWeldment()) return;

  bool didSplit =
    leftModule->GetWeldment()->SplitIfAdjacent(leftModule, rightModule);
  if (didSplit) { // re-trigger EngageGrab to reset center on hand that was already grabbing
    AVRMotionController* controller = GetControllerForHand(AlreadyGrabbingHand);
    AGrabbableActor* grabbable =
      AlreadyGrabbingHand == EControllerHand::Left
        ? LeftHandGrabbableActor
        : RightHandGrabbableActor;

    grabbable->EngageGrab(
      controller->GetActorLocation(),
      controller->GetActorRotation()
    );
  }
}

void AVRAvatar::HandleStartGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AGrabbableActor* grabbedActor =
    Hand == EControllerHand::Left
      ? LeftHandGrabbableActor
      : RightHandGrabbableActor;

  if (GetControllerForOtherHand(Hand)->IsGrabbing())
    MaybeSplitWeldment(GetOtherHand(Hand));

  controller->StartGrab();
  grabbedActor->EngageGrab(controller->GetActorLocation(), controller->GetActorRotation());
}

void AVRAvatar::HandleGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AGrabbableActor* grabbedActor =
    Hand == EControllerHand::Left
      ? LeftHandGrabbableActor
      : RightHandGrabbableActor;

  grabbedActor->AlterGrab(controller->GetActorLocation(), controller->GetActorRotation());
}

void AVRAvatar::HandleCompleteGrab(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AGrabbableActor* grabbedActor =
    Hand == EControllerHand::Left
      ? LeftHandGrabbableActor
      : RightHandGrabbableActor;

  grabbedActor->ReleaseGrab();
  controller->EndGrab();

  // if we snapped to another module/weldment that was currently being grabbed
  // re-trigger EngageGrab to reset center on snapped to actor
  if (GetControllerForOtherHand(Hand)->IsGrabbing()) {
    AGrabbableActor* otherGrabbable =
      Hand == EControllerHand::Left
        ? RightHandGrabbableActor
        : LeftHandGrabbableActor;
    if (grabbedActor->IsInWeldment() && grabbedActor->GetWeldment()->Contains(otherGrabbable)) {
      AVRMotionController* otherController = GetControllerForOtherHand(Hand);
      otherGrabbable->EngageGrab(
        otherController->GetActorLocation(),
        otherController->GetActorRotation()
      );
    }
  }
}

void AVRAvatar::HandleSnapModeTriggered(const FInputActionValue& _Value, EControllerHand Hand) {
  AGrabbableActor* grabbedActor =
    Hand == EControllerHand::Left
      ? LeftHandGrabbableActor
      : RightHandGrabbableActor;

  if (Cast<AVCVModule>(grabbedActor))
    Cast<AVCVModule>(grabbedActor)->InitSnapMode();
}

void AVRAvatar::HandleContextMenuTriggeredOrSnapModeCancelled(const FInputActionValue& _Value, EControllerHand Hand) {
  AGrabbableActor* grabbedActor =
    Hand == EControllerHand::Left
      ? LeftHandGrabbableActor
      : RightHandGrabbableActor;

  AVCVModule* grabbedModule = Cast<AVCVModule>(grabbedActor);
  if (grabbedModule) {
    if (grabbedModule->IsInSnapMode()) {
      grabbedModule->CancelSnapMode();

      // re-engage to reset centering and control weldment instead
      if (grabbedModule->IsInWeldment()) {
        AVRMotionController* controller = GetControllerForHand(Hand);
        grabbedModule->EngageGrab(controller->GetActorLocation(), controller->GetActorRotation());
      }
    } else {
      grabbedModule->ToggleContextMenu();
    }
  }
}

void AVRAvatar::HandleDuplicateModule(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVModule* grabbedModule =
    Cast<AVCVModule>(
      Hand == EControllerHand::Left
        ? LeftHandGrabbableActor
        : RightHandGrabbableActor
    );

  if (grabbedModule) GameMode->DuplicateModule(grabbedModule);
}

void AVRAvatar::HandleDestroyModule(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVModule* grabbedModule =
    Cast<AVCVModule>(
      Hand == EControllerHand::Left
        ? LeftHandGrabbableActor
        : RightHandGrabbableActor
    );

  if (grabbedModule) {
    if (grabbedModule->IsInWeldment()) {
      GameMode->SplitWeldment(grabbedModule->GetWeldment(), grabbedModule);
      // re-engage to fix module positioning
      grabbedModule->EngageGrab(
        controller->GetActorLocation(),
        controller->GetActorRotation()
      );
    } else {
      GameMode->DestroyModule(grabbedModule->Id);
    }
    return;
  }

  ALibrary* library =
    Cast<ALibrary>(
      Hand == EControllerHand::Left
        ? LeftHandGrabbableActor
        : RightHandGrabbableActor
    );
  if (library) GameMode->TuckLibrary();
}

void AVRAvatar::HandleStartParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVParam* interactingParam =
    Hand == EControllerHand::Left
      ? LeftHandParamActor
      : RightHandParamActor;

  controller->StartParamInteract();

  if (Cast<AVCVKnob>(interactingParam)) {
    interactingParam->Engage(controller->GetActorRotation().Roll);
  } else if (Cast<AVCVSlider>(interactingParam)) {
    interactingParam->Engage(controller->GetActorLocation());
  } else {
    interactingParam->Engage();
  }
}

void AVRAvatar::HandleParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVParam* interactingParam =
    Hand == EControllerHand::Left
      ? LeftHandParamActor
      : RightHandParamActor;

  if (Cast<AVCVKnob>(interactingParam)) {
    interactingParam->Alter(controller->GetActorRotation().Roll);
  } else if (Cast<AVCVSlider>(interactingParam)) {
    interactingParam->Alter(controller->GetActorLocation());
  }

  controller->RefreshTooltip();
}

void AVRAvatar::HandleParamReset(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVParam* interactingParam =
    Hand == EControllerHand::Left
      ? LeftHandParamActor
      : RightHandParamActor;

  interactingParam->Release();
  interactingParam->ResetValue();
  controller->EndParamInteract();
}

void AVRAvatar::HandleCompleteParamEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  AVCVParam* interactingParam =
    Hand == EControllerHand::Left
      ? LeftHandParamActor
      : RightHandParamActor;

  controller->EndParamInteract();
  interactingParam->Release();
}

void AVRAvatar::HandleStartPortEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  controller->StartPortInteract();
}

void AVRAvatar::HandlePortEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  if (!heldCableEnd) return;

  FVector location, forwardVector;
  controller->GetHeldCableEndInfo(location, forwardVector);
  heldCableEnd->SetPosition(location, forwardVector.Rotation());
}

void AVRAvatar::HandleCompletePortEngage(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  bool connected = heldCableEnd->Drop();
  controller->EndPortInteract(connected);
}

void AVRAvatar::HandleCableDestroy(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  if (!heldCableEnd) return;

  if (heldCableEnd->Cable->IsLatched()) heldCableEnd->Cable->ToggleLatched();
  heldCableEnd->Drop();
  controller->EndPortInteract(false);
}

void AVRAvatar::HandleCableLatch(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  if (heldCableEnd) heldCableEnd->Cable->ToggleLatched();
}

void AVRAvatar::HandleCableColorCycle(const FInputActionValue& Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  int& cycleDirection =
    Hand == EControllerHand::Left
      ? LeftCableColorCycleDirection
      : RightCableColorCycleDirection;

  if (Value.Get<float>() > 0.8f) cycleDirection = 1;
  if (Value.Get<float>() < -0.8f) cycleDirection = -1;
}
void AVRAvatar::HandleCompleteCableColorCycle(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  ACableEnd* heldCableEnd =
    Hand == EControllerHand::Left
      ? LeftHandHeldCableEnd
      : RightHandHeldCableEnd;

  int& cycleDirection =
    Hand == EControllerHand::Left
      ? LeftCableColorCycleDirection
      : RightCableColorCycleDirection;

  if (heldCableEnd) heldCableEnd->Cable->CycleColor(cycleDirection);
  cycleDirection = 0;
}

void AVRAvatar::RequestScreenshot(const FInputActionValue& _Value) {
  if (bLeftHandWorldManipulationActive || bRightHandWorldManipulationActive) return;

  // Generate a filename based on the current date
  const FDateTime Now = FDateTime::Now();
  // store screenshot in Project directory next to main UProject/EXE based on the build type
#if WITH_EDITOR
  const FString ImageDirectory = FString::Printf(TEXT("%s/%s"), *FPaths::ProjectDir(), TEXT("Screenshots"));
#else
  const FString ImageDirectory = FString::Printf(TEXT("%s/../%s"), *FPaths::ProjectDir(), TEXT("Screenshots"));
#endif
  const FString ImageFilename = FString::Printf(TEXT("%s/VCVRVRScreenshot_%d%02d%02d_%02d%02d%02d_%03d.png"), *ImageDirectory, Now.GetYear(), Now.GetMonth(), Now.GetDay(), Now.GetHour(), Now.GetMinute(), Now.GetSecond(), Now.GetMillisecond());
  FScreenshotRequest::RequestScreenshot(ImageFilename, false, false);
}

void AVRAvatar::Quit(const FInputActionValue& _Value) {
  GameMode->RequestExit();
}

void AVRAvatar::ToggleMainMenu(const FInputActionValue& _Value) {
  GameMode->ToggleMainMenu();
}

void AVRAvatar::SummonLibrary(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  FVector location = controller->GetActorLocation() + controller->GetActorForwardVector() * 16.f;
  FRotator rotation =
    UKismetMathLibrary::FindLookAtRotation(
      Camera->GetComponentLocation(),
      location
    );
  GameMode->SummonLibrary(location, rotation);
}

void AVRAvatar::HandleStartWidgetLeftClick(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  controller->StartWidgetLeftClick();
}

void AVRAvatar::HandleCompleteWidgetLeftClick(const FInputActionValue& _Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  controller->EndWidgetLeftClick();
}

void AVRAvatar::HandleWidgetScroll(const FInputActionValue& Value, EControllerHand Hand) {
  AVRMotionController* controller = GetControllerForHand(Hand);
  controller->WidgetScroll(Value.Get<float>());
}

// get the position+rotation for spawning a WidgetSurrogate in the camera's view
// so it is invisible (facing away) but renders immediately
void AVRAvatar::GetRenderablePosition(FVector& Location, FRotator& Rotation) {
  Location = Camera->GetComponentLocation() + Camera->GetForwardVector() * 10.f;
  Rotation = Camera->GetForwardVector().Rotation();
}

FRotator AVRAvatar::GetLookAtCameraRotation(FVector FromPosition) {
  return UKismetMathLibrary::FindLookAtRotation(
    FromPosition,
    Camera->GetComponentLocation()
  );
}

FVector AVRAvatar::GetMainMenuPosition() {
  return Camera->GetComponentLocation() + Camera->GetForwardVector() * 60;
}

