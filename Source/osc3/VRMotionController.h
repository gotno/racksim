#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MotionControllerComponent.h"

#include "VRMotionController.generated.h"

class USphereComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
class UWidgetComponent;
class UTooltip;
class UUserWidget;
class Aosc3GameModeBase;
class APlayerController;
class AVRAvatar;
class AVCVCable;
class AVCVPort;

// USTRUCT()
// struct FTooltipWidgetClasses {
// 	GENERATED_BODY()

//   UPROPERTY(EditDefaultsOnly)
//   TSubclassOf<UUserWidget> OneLine;

//   UPROPERTY(EditDefaultsOnly)
//   TSubclassOf<UUserWidget> TwoLines;

//   UPROPERTY(EditDefaultsOnly)
//   TSubclassOf<UUserWidget> TwoLinesWithEmphasis;

//   UPROPERTY(EditDefaultsOnly)
//   TSubclassOf<UUserWidget> TwoLinesWithSub;
// };

// USTRUCT()
// struct FTooltipWidgets {
// 	GENERATED_BODY()

//   UPROPERTY()
//   UTooltip* OneLine;
//   UPROPERTY()
//   UTooltip* TwoLines;
//   UPROPERTY()
//   UTooltip* TwoLinesWithEmphasis;
//   UPROPERTY()
//   UTooltip* TwoLinesWithSub;
// };

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
  AVCVPort* GetPortActorToInteract() { return OriginPortActor; }
  AVCVPort* GetDestinationPortActor() { return DestinationPortActor; }
  AVCVCable* GetHeldCable() { return HeldCable; }
  void StartGrab();
  void EndGrab();
  void StartParamInteract();
  void EndParamInteract();
  void StartPortInteract();
  void EndPortInteract();

  void UpdateTooltip();

  UPROPERTY(EditAnywhere, Category="Input")
  FHapticEffects HapticEffects;
  // UPROPERTY(EditDefaultsOnly, Category="Tooltip")
  // FTooltipWidgetClasses TooltipWidgetClasses;
  
private:
  AVRAvatar* Avatar;
  Aosc3GameModeBase* GameMode;
  APlayerController* PlayerController;

  FString HandName;

  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* TooltipWidgetComponent;
  // UPROPERTY()
  // FTooltipWidgets TooltipWidgets;
  UTooltip* TooltipWidget;

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* MotionController;
  
  UPROPERTY(VisibleAnywhere)
  USphereComponent* GrabSphere;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float GrabSphereRadius{2.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float GrabSphereUpOffset{-2.f};

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

  bool bIsParamInteracting{false};
  AActor* ParamActorToInteract;

  bool bIsPortInteracting{false};
  AVCVPort* PortActorToInteract;
  AVCVPort* OriginPortActor;
  AVCVPort* DestinationPortActor;
  AVCVCable* HeldCable;
};
