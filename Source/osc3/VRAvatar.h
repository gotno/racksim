#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"

#include "VRAvatar.generated.h"

class UMotionControllerComponent;
class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
struct FHitResult;

USTRUCT()
struct FInputActions {
	GENERATED_BODY()
    
  UPROPERTY(EditDefaultsOnly)
  UInputAction* Teleport;

  UPROPERTY(EditDefaultsOnly)
  UInputAction* TranslateWorld;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* RotateWorldRight;
  UPROPERTY(EditDefaultsOnly)
  UInputAction* RotateWorldLeft;

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
  FInputActions InputActions;
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

  void StartTeleport(const FInputActionValue& _Value);
  void SweepDestination(const FInputActionValue& _Value);
  void CompleteTeleport(const FInputActionValue& _Value);
  FHitResult DestinationHitResult;
  bool HasDestinationHit;
  FVector LastDestinationLocation{FVector::ZeroVector};
  bool bTeleportActive{false};

  void StartRotateWorldLeft(const FInputActionValue& _Value);
  void RotateWorldLeft(const FInputActionValue& _Value);
  // bool bRotateWorldLeftActive{false};
  void StartRotateWorldRight(const FInputActionValue& _Value);
  void RotateWorldRight(const FInputActionValue& _Value);
  // bool bRotateWorldRightActive{false};
  void RotateWorld(float degrees);
  void CompleteRotateWorld(const FInputActionValue& _Value);
  float LastRotateWorldDegrees{0.f};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float RotateWorldScale{2.f};

  void StartTranslateWorld(const FInputActionValue& _Value);
  void CompleteTranslateWorld(const FInputActionValue& _Value);
  bool bWorldTranslateActive{false};
  void TranslateWorld(const FInputActionValue& _Value);
  FVector LastTranslateWorldOffset{FVector::ZeroVector};

  UPROPERTY(EditDefaultsOnly, Category="Input")
  float TranslateWorldScale{1.f};

  void Quit(const FInputActionValue& _Value);

  FVector LastLeftHandLocation{FVector::ZeroVector};
  FVector LastRightHandLocation{FVector::ZeroVector};
  
};