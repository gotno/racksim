#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MotionControllerComponent.h"

#include "VRMotionController.generated.h"

class USphereComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
class APlayerController;

USTRUCT()
struct FHapticEffects {
	GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UHapticFeedbackEffect_Base* Bump;
};

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

  AActor* GetActorToGrab() { return ActorToGrab; }
  AActor* GetParamActorToInteract() { return ParamActorToInteract; }
  AActor* GetPortActorToInteract() { return PortActorToInteract; }
  void StartGrab();
  void EndGrab();
  void StartParamInteract();
  void EndParamInteract();

  UPROPERTY(EditAnywhere, Category="Input")
  FHapticEffects HapticEffects;
  
private:
  FString HandName;

  APlayerController* PlayerController;

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* MotionController;
  
  UPROPERTY(VisibleAnywhere)
  USphereComponent* GrabSphere;
  float GrabSphereRadius{2.f};

  UPROPERTY(VisibleAnywhere)
  UCapsuleComponent* InteractCapsule;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleRadius{0.2f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleHalfHeight{3.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleForwardOffset{1.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleAngleOffset{0.f};
  
  UFUNCTION()
  void LogOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

  UFUNCTION()
  void HandleGrabberBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
  UFUNCTION()
  void HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
  AActor* ActorToGrab;
  bool bIsGrabbing{false};

  UFUNCTION()
  void HandleInteractorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
  UFUNCTION()
  void HandleInteractorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
  bool bIsInteracting{false};
  AActor* ParamActorToInteract;
  AActor* PortActorToInteract;
};
