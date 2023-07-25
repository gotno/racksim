#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Avatar.generated.h"

UCLASS()
class OSC3_API AAvatar : public APawn {
	GENERATED_BODY()

public:
	AAvatar();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
  void GetRenderablePosition(FVector& location, FRotator& rotation);
private:
  UPROPERTY(EditAnywhere)
  class UCameraComponent* cameraComponent; 
  UPROPERTY(EditAnywhere)
  class UFloatingPawnMovement* movementComponent;
  
  class Aosc3GameModeBase* gameMode;
  
  FHitResult hitResult;
  AActor* hitActor;
  bool hasHit = false;
  float interactDistance = 40.f;
  void sweep();
  void drawIndicator();

  void moveForward(float axisValue);
  void moveRight(float axisValue);
  void moveUp(float axisValue);
  
  void click();
  void release();
  float clickMouseX;
  float clickMouseY;
  
  class AVCVParam* controlledParam;
  class AVCVCable* controlledCable;
  
  bool hovering = false;
  
  FColor indicatorColor{FColor::Black};
  FColor indicatorColorHover{FColor::Turquoise};

  virtual void AddControllerPitchInput(float axisValue) override;
  virtual void AddControllerYawInput(float axisValue) override;
};
