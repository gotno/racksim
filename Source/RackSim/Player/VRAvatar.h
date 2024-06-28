#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"

#include "VRAvatar.generated.h"

class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
class AVRMotionController;
struct FHitResult;
class UHapticFeedbackEffect_Base;
class Aosc3GameModeBase;
class Aosc3GameState;
class AVCVCable;
class ACableEnd;
class AVCVParam;
class AVCVPort;
class AGrabbableActor;

USTRUCT()
struct FBaseActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* RequestScreenshot;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* Quit;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* ToggleMenu;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* SummonLibraryLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* SummonLibraryRight;
};

USTRUCT()
struct FWidgetManipulationActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* WidgetLeftClickLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* WidgetLeftClickRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* WidgetScrollLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* WidgetScrollRight;
};

USTRUCT()
struct FWorldManipulationActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* TranslateWorldLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* TranslateWorldRight;
  
  UPROPERTY(EditDefaultsOnly)
  UInputAction* RotateWorldLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* RotateWorldRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* RotoTranslateWorld;
    
  UPROPERTY(EditDefaultsOnly)
  UInputAction* TeleportLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* TeleportRight;
};

USTRUCT()
struct FModuleManipulationActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* GrabLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* GrabRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* DuplicateModuleLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* DuplicateModuleRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* DestroyModuleLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* DestroyModuleRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* ModuleContextMenuOrSnapModeLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* ModuleContextMenuOrSnapModeRight;
};

USTRUCT()
struct FParamInteractionActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamEngageLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamEngageRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamResetLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamResetRight;
};

USTRUCT()
struct FPortInteractionActions {
  GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* PortEngageLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* PortEngageRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableDestroyLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableDestroyRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableLatchLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableLatchRight;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableColorCycleLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* CableColorCycleRight;
};

USTRUCT()
struct FInputMappingContexts {
	GENERATED_BODY()
    
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* Base;

  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* WidgetManipulationLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* WidgetManipulationRight;

  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* ModuleManipulationLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* ModuleManipulationRight;
    
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* WorldManipulationLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* WorldManipulationRight;

  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* ParamInteractionLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* ParamInteractionRight;

  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* PortInteractionLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* PortInteractionRight;
};

class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class RACKSIM_API AVRAvatar : public ACharacter {
  GENERATED_BODY()

public:
  AVRAvatar();

protected:
  virtual void BeginPlay() override;
  virtual void PawnClientRestart() override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
  void EnableWorldManipulation();

  AVRMotionController* GetMotionController(EControllerHand Hand) {
    if (Hand == EControllerHand::Left) {
      return LeftController;
    } else {
      return RightController;
    }
  }

  void GetSavegamePlayerPosition(FVector& Location, FRotator& Rotation);
  void SetPlayerPosition(FVector& Location, FRotator& Rotation);

  UPROPERTY(EditAnywhere, Category="Input")
  FBaseActions BaseActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FWidgetManipulationActions WidgetManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FWorldManipulationActions WorldManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FModuleManipulationActions ModuleManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FParamInteractionActions ParamInteractionActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FPortInteractionActions PortInteractionActions;

  UPROPERTY(EditAnywhere, Category="Input")
  FInputMappingContexts InputMappingContexts;

  UEnhancedInputLocalPlayerSubsystem* InputSubsystem;
  
  void SetControllerWidgetInteracting(EControllerHand Hand, bool bEnable);
  void SetControllerGrabbing(EControllerHand Hand, bool bEnable);
  void SetControllerParamInteracting(EControllerHand Hand, bool bEnable);
  void SetControllerPortInteracting(EControllerHand Hand, bool bEnable);
  
  void GetRenderablePosition(FVector& Location, FRotator& Rotation);
  FRotator GetLookAtCameraRotation(FVector FromLocation);
  FVector GetMainMenuPosition();

private:
  UPROPERTY(VisibleAnywhere)
  UCameraComponent* Camera;

  UPROPERTY(EditDefaultsOnly, Category="Input")
  TSubclassOf<AVRMotionController> MotionControllerClass;
  UPROPERTY(VisibleAnywhere)
  AVRMotionController* LeftController{nullptr};
  UPROPERTY(VisibleAnywhere)
  AVRMotionController* RightController{nullptr};

  UPROPERTY(VisibleAnywhere)
  USceneComponent* VRRoot;

  UPROPERTY(VisibleAnywhere)
  AActor* DestinationMarker;
  UPROPERTY(EditDefaultsOnly, Category="Input")
  TSubclassOf<AActor> DestinationMarkerClass;
  
  Aosc3GameModeBase* GameMode{nullptr};
  Aosc3GameState* GameState{nullptr};

  EControllerHand GetOtherHand(EControllerHand Hand) {
    return Hand == EControllerHand::Left ? EControllerHand::Right : EControllerHand::Left;
  }
  AVRMotionController* GetControllerForHand(EControllerHand Hand) {
    return Hand == EControllerHand::Left ? LeftController : RightController;
  }
  AVRMotionController* GetControllerForOtherHand(EControllerHand Hand) {
    return Hand == EControllerHand::Left ? RightController : LeftController;
  }

  // world manipulation
  bool bLeftHandWorldManipulationActive{false};
  bool bRightHandWorldManipulationActive{false};
  void SetWorldManipulationActive(EControllerHand Hand, bool bActive);

  FVector LastLeftHandLocation{FVector::ZeroVector};
  FVector LastRightHandLocation{FVector::ZeroVector};

  void HandleStartTeleport(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleTeleport(const FInputActionValue& Value, EControllerHand Hand);
  void HandleCompleteTeleport(const FInputActionValue& _Value, EControllerHand Hand);
  void SweepDestination(EControllerHand Hand);
  FHitResult DestinationHitResult;
  bool HasDestinationHit;
  FVector LastDestinationLocation{FVector::ZeroVector};

  float GetRotationalDistanceBetweenControllerPositions(const FVector& c1, const FVector& c2);
  void HandleStartRotateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleRotateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  void RotateWorldAroundPivot(float degrees, FVector pivot);
  void HandleCompleteRotateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  float LastRotateWorldDelta{0.f};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float RotateWorldScale{1.f};

  void HandleStartTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteTranslateWorld(const FInputActionValue& _Value, EControllerHand Hand);
  FVector LastTranslateWorldDelta{FVector::ZeroVector};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float TranslateWorldScale{1.f};

  void HandleStartRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value);

  // module manipulation
  AGrabbableActor* LeftHandGrabbableActor{nullptr};
  AGrabbableActor* RightHandGrabbableActor{nullptr};
  AGrabbableActor* LeftHandGrabbedActor{nullptr};
  AGrabbableActor* RightHandGrabbedActor{nullptr};
  void HandleStartGrab(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleGrab(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteGrab(const FInputActionValue& _Value, EControllerHand Hand);
  void MaybeSplitWeldment(EControllerHand AlreadyGrabbingHand);

  void HandleSnapModeTriggered(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleContextMenuTriggeredOrSnapModeCancelled(const FInputActionValue& _Value, EControllerHand Hand);

  void HandleDuplicateModule(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleDestroyModule(const FInputActionValue& _Value, EControllerHand Hand);

  // param interaction
  AVCVParam* LeftHandParamActor{nullptr};
  AVCVParam* RightHandParamActor{nullptr};
  void HandleStartParamEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleParamEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteParamEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleParamReset(const FInputActionValue& _Value, EControllerHand Hand);

  ACableEnd* LeftHandHeldCableEnd;
  ACableEnd* RightHandHeldCableEnd;
  void HandleStartPortEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandlePortEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompletePortEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCableDestroy(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCableLatch(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCableColorCycle(const FInputActionValue& Value, EControllerHand Hand);
  void HandleCompleteCableColorCycle(const FInputActionValue& _Value, EControllerHand Hand);
  int LeftCableColorCycleDirection{0};
  int RightCableColorCycleDirection{0};

  void LogInput(const FInputActionValue& _Value, FString msg);

  // general controls
  void RequestScreenshot(const FInputActionValue& _Value);
  void Quit(const FInputActionValue& _Value);
  void ToggleMainMenu(const FInputActionValue& _Value);
  void SummonLibrary(const FInputActionValue& _Value, EControllerHand Hand);
  
  // widget controls
  void HandleStartWidgetLeftClick(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteWidgetLeftClick(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleWidgetScroll(const FInputActionValue& _Value, EControllerHand Hand);

public:
  // delegate stuff
  void HandleGrabbableTargetSet(AActor* GrabbableActor, EControllerHand Hand);
  void HandleParamTargetSet(AActor* ParamActor, EControllerHand Hand);
  void HandleOriginPortOrCableTargetSet(AActor* PortOrCableActor, EControllerHand Hand);
  void HandleDestinationPortTargetSet(AActor* PortActor, EControllerHand Hand);
  void HandleHeldCableSet(AActor* CableEnd, EControllerHand Hand);
};