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

USTRUCT()
struct FBaseActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* Quit;
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
};

USTRUCT()
struct FParamInteractionActions {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamEngageLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* ParamEngageRight;
};

USTRUCT()
struct FInputMappingContexts {
	GENERATED_BODY()
    
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* Base;

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
};

class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class OSC3_API AVRAvatar : public ACharacter {
	GENERATED_BODY()

public:
	AVRAvatar();

protected:
	virtual void BeginPlay() override;
  virtual void PawnClientRestart() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

  UPROPERTY(EditAnywhere, Category="Input")
  FBaseActions BaseActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FWorldManipulationActions WorldManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FModuleManipulationActions ModuleManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FParamInteractionActions ParamInteractionActions;

  UPROPERTY(EditAnywhere, Category="Input")
  FInputMappingContexts InputMappingContexts;

  UEnhancedInputLocalPlayerSubsystem* InputSubsystem;
  
  void SetControllerGrabbing(EControllerHand Hand, bool bGrabbing);
  void SetControllerParamInteracting(EControllerHand Hand, bool bInteracting);
  
  void GetRenderablePosition(FVector& location, FRotator& rotation);

private:
  UPROPERTY(VisibleAnywhere)
  UCameraComponent* Camera;

  UPROPERTY(EditDefaultsOnly, Category="Input")
  TSubclassOf<AVRMotionController> MotionControllerClass;
  UPROPERTY(VisibleAnywhere)
  AVRMotionController* LeftController;
  UPROPERTY(VisibleAnywhere)
  AVRMotionController* RightController;

  UPROPERTY(VisibleAnywhere)
  USceneComponent* VRRoot;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* DestinationMarker;

  // world manipulation
  bool bLeftHandWorldManipulationActive{false};
  bool bRightHandWorldManipulationActive{false};
  void SetWorldManipulationActive(EControllerHand Hand, bool Active);

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

  bool bLeftHandRotoTranslateActive{false};
  void HandleStartRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value);

  // module manipulation
  void HandleStartGrab(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleGrab(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteGrab(const FInputActionValue& _Value, EControllerHand Hand);

  // param interaction
  void HandleStartParamEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleParamEngage(const FInputActionValue& _Value, EControllerHand Hand);
  void HandleCompleteParamEngage(const FInputActionValue& _Value, EControllerHand Hand);

  void LogInput(const FInputActionValue& _Value, FString msg);

  // general controls
  void Quit(const FInputActionValue& _Value);
};