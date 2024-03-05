#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

#include "VRMotionController.generated.h"

class Aosc3GameModeBase;
class APlayerController;
class AVRAvatar;
class AVCVCable;
class AVCVPort;
class UTooltip;

class UMotionControllerComponent;
class USceneComponent;
class USphereComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
class UWidgetComponent;
class UWidgetInteractionComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGrabbableTargetedSignature, AActor* /* GrabbableActor */, EControllerHand /* Hand */);

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

  void SetTrackingSource(EControllerHand Hand);

  void StartGrab();
  void EndGrab();

  AActor* GetParamActorToInteract() { return ParamActorToInteract; }
  void StartParamInteract();
  void EndParamInteract();

  AVCVPort* GetPortActorToInteract() { return OriginPortActor; }
  AVCVPort* GetDestinationPortActor() { return DestinationPortActor; }
  AVCVCable* GetHeldCable() { return HeldCable; }
  void StartPortInteract();
  void EndPortInteract();
  void GetHeldCableEndInfo(FVector& Location, FVector& ForwardVector);

  void StartWidgetLeftClick();
  void EndWidgetLeftClick();
  void WidgetScroll(float ScrollDelta);

  void RefreshTooltip();

  UPROPERTY(EditAnywhere, Category="Input")
  FHapticEffects HapticEffects;
  
private:
  AVRAvatar* Avatar;
  Aosc3GameModeBase* GameMode;
  APlayerController* PlayerController;

  FString HandName;

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* MotionController;
  
  UPROPERTY(VisibleAnywhere)
  UWidgetInteractionComponent* WidgetInteractionComponent;

  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* TooltipWidgetComponent;
  UTooltip* TooltipWidget;
  
  // interactor
  UPROPERTY(VisibleAnywhere)
  USceneComponent* InteractCapsuleRoot;
  UPROPERTY(VisibleAnywhere)
  UCapsuleComponent* InteractCapsule;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleRadius{0.2f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractCapsuleHalfHeight{2.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  FVector InteractCapsuleOffset{0.f, 0.f, 0.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  FRotator InteractCapsuleRotation{0.f, -90.f, 0.f};

  UFUNCTION()
  void HandleInteractorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
  UFUNCTION()
  void HandleInteractorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

  // grabber
  UPROPERTY(VisibleAnywhere)
  USphereComponent* GrabSphere;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float GrabSphereRadius = MODULE_DEPTH * RENDER_SCALE;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  FVector GrabSphereOffset{0.f, -2.f, -2.f};

  void GrabberTick();
  UFUNCTION()
  void HandleGrabberEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
  bool bIsGrabbing{false};

  bool bIsParamInteracting{false};
  AActor* ParamActorToInteract;
  
  UFUNCTION()
  void LogOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

  void SetTooltipVisibility(bool bVisible);
  // TODO: user setting
  bool bTooltipEnabled{true};

  bool bIsPortInteracting{false};
  AVCVPort* PortActorToInteract;
  AVCVPort* OriginPortActor;
  AVCVPort* DestinationPortActor;
  AVCVCable* HeldCable;

  bool bIsWidgetInteracting{false};

  public:
    // delegates
    FOnGrabbableTargetedSignature OnGrabbableTargetedDelegate;
};
