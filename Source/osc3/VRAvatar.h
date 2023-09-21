#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "MotionControllerComponent.h"

#include "VRAvatar.generated.h"

class UMotionControllerComponent;
class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
struct FHitResult;

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

  UPROPERTY(EditDefaultsOnly)
  UInputAction* Quit;
};

USTRUCT()
struct FInputMappingContexts {
	GENERATED_BODY()
    
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* WorldManipulation;
    
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* RotateWorldLeft;
  UPROPERTY(EditDefaultsOnly)
  UInputMappingContext* RotateWorldRight;
};

class UEnhancedInputLocalPlayerSubsystem;
UCLASS()
class OSC3_API AVRAvatar : public ACharacter {
	GENERATED_BODY()

public:
	AVRAvatar();
  
  UPROPERTY(EditAnywhere, Category="Input")
  FWorldManipulationActions WorldManipulationActions;
  UPROPERTY(EditAnywhere, Category="Input")
  FInputMappingContexts InputMappingContexts;

  UEnhancedInputLocalPlayerSubsystem* InputSubsystem;
  
  float GetCameraHeight();
  void GetRenderablePosition(FVector& location, FRotator& rotation);

protected:
	virtual void BeginPlay() override;
  virtual void PawnClientRestart() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
  UPROPERTY(VisibleAnywhere)
  UCameraComponent* Camera;

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* LeftController;
  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* RightController;

  UPROPERTY(VisibleAnywhere)
  USceneComponent* VRRoot;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* DestinationMarker;
  
  float GetRotationalDistanceBetweenControllerPositions(const FVector& c1, const FVector& c2);
  
  void MoveForward(float throttle);
  void MoveRight(float throttle);

  void HandleStartTeleport(const FInputActionValue& _Value);
  void HandleTeleport(const FInputActionValue& Value, EControllerHand hand);
  void HandleCompleteTeleport(const FInputActionValue& _Value);
  void SweepDestination(EControllerHand hand);
  FHitResult DestinationHitResult;
  bool HasDestinationHit;
  FVector LastDestinationLocation{FVector::ZeroVector};

  void HandleStartRotateWorld(const FInputActionValue& _Value, EControllerHand hand);
  void HandleRotateWorld(const FInputActionValue& _Value, EControllerHand hand);
  void RotateWorldAroundPivot(float degrees, FVector pivot);
  void HandleCompleteRotateWorld(const FInputActionValue& _Value);
  float LastRotateWorldDelta{0.f};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float RotateWorldScale{1.f};

  void HandleStartTranslateWorld(const FInputActionValue& _Value, EControllerHand hand);
  void HandleTranslateWorld(const FInputActionValue& _Value, EControllerHand hand);
  void HandleCompleteTranslateWorld(const FInputActionValue& _Value);
  FVector LastTranslateWorldDelta{FVector::ZeroVector};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float TranslateWorldScale{1.f};

  void HandleStartRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleRotoTranslateWorld(const FInputActionValue& _Value);
  void HandleCompleteRotoTranslateWorld(const FInputActionValue& _Value);

  void LogInput(const FInputActionValue& _Value, FString msg);

  void Quit(const FInputActionValue& _Value);

  FVector LastLeftHandLocation{FVector::ZeroVector};
  FVector LastRightHandLocation{FVector::ZeroVector};
  
};