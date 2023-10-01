#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MotionControllerComponent.h"

#include "VRMotionController.generated.h"

class USphereComponent;
class UPrimitiveComponent;

UCLASS()
class OSC3_API AVRMotionController : public AActor {
	GENERATED_BODY()
	
public:	
	AVRMotionController();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void SetTrackingSource(EControllerHand Hand) {
    MotionController->SetTrackingSource(Hand);

    HandName = Hand == EControllerHand::Left ? "left" : "right";
  }

  AActor* GetActorToGrab();
  void StartGrab();
  void EndGrab();
  
private:
  FString HandName;
  AActor* ActorToGrab;
  bool bIsGrabbing{false};

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* MotionController;
  
  UPROPERTY(VisibleAnywhere)
  USphereComponent* GrabSphere;
  
  UFUNCTION()
  void LogOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
  UFUNCTION()
  void HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
  UFUNCTION()
  void HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
