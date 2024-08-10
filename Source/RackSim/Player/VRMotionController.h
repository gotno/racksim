#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

#include "VRMotionController.generated.h"

class Aosc3GameModeBase;
class APlayerController;
class AVRAvatar;
class AVCVCable;
class ACableEnd;
class AVCVPort;
class UTooltip;

class UMotionControllerComponent;
class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UCapsuleComponent;
class UPrimitiveComponent;
class UWidgetComponent;
class UPointLightComponent;
class UWidgetInteractionComponent;
class UNiagaraComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGrabbableTargetedSignature, AActor* /* GrabbableActor */, EControllerHand /* Hand */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnParamTargetedSignature, AActor* /* VCVParam */, EControllerHand /* Hand */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnOriginPortTargetedSignature, AActor* /* VCVPort */, EControllerHand /* Hand */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCableTargetedSignature, AActor* /* VCVPort */, EControllerHand /* Hand */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCableHeldSignature, AActor* /* VCVCable */, EControllerHand /* Hand */);

USTRUCT()
struct FHapticEffects {
  GENERATED_BODY()

  UPROPERTY(EditDefaultsOnly)
  UHapticFeedbackEffect_Base* Bump;
  UPROPERTY(EditDefaultsOnly)
  UHapticFeedbackEffect_Base* Thump;
};

UCLASS()
class RACKSIM_API AVRMotionController : public AActor {
  GENERATED_BODY()

public:
  AVRMotionController();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;

  void SetTrackingSource(EControllerHand Hand);

  void SetWorldInteract(bool bActive);

  void StartGrab();
  void EndGrab();
  bool IsGrabbing() { return bIsGrabbing; }

  void StartParamInteract();
  void EndParamInteract();

  void StartPortInteract();
  void EndPortInteract(bool Connected);
  void GetHeldCableEndInfo(FVector& Location, FVector& ForwardVector);

  void StartWidgetLeftClick();
  void EndWidgetLeftClick();
  void WidgetScroll(float ScrollDelta);

  void RefreshTooltip();

  void SetLightHidden(bool bLightHidden);
  void SetTooltipHidden(bool bTooltipHidden);

  UPROPERTY(EditAnywhere, Category="Input")
  FHapticEffects HapticEffects;

private:
  AVRAvatar* Avatar;
  Aosc3GameModeBase* GameMode;
  APlayerController* PlayerController;
  FString HandName;

  UPROPERTY(VisibleAnywhere)
  UNiagaraComponent* PointerFXComponent;

  UPROPERTY(VisibleAnywhere)
  UMotionControllerComponent* MotionController;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* ControllerMesh;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* InteractIndicatorMesh;

  // grabber
  UPROPERTY(VisibleAnywhere)
  USphereComponent* GrabSphere;
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float GrabSphereRadius = UNSCALED_MODULE_DEPTH * 2.5;

  UPROPERTY(VisibleAnywhere)
  UPointLightComponent* Light;

  UPROPERTY(VisibleAnywhere)
  UWidgetInteractionComponent* WidgetInteractionComponent;

  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* TooltipWidgetComponent;
  UTooltip* TooltipWidget;
  void WidgetInteractionTick();
  
  void HapticBump();
  void HapticThump();

  void SetTooltipVisibility(bool bVisible);
  // TODO: user setting
  bool bTooltipEnabled{true};

  bool bIsWorldInteracting{false};

  FVector InteractTraceStart{0.f}, InteractTraceEnd{0.f};
  UPROPERTY(EditDefaultsOnly, Category="Interaction")
  float InteractTraceReach{4.f};

  void ParamTargetTick();
  AActor* TargetedParam{nullptr};
  bool bIsParamInteracting{false};

  bool bIsGrabbing{false};
  void GrabbableTargetTick();
  UPROPERTY()
  AActor* TargetedGrabbable{nullptr};

  bool bIsPortInteracting{false};
  void PortTargetTick();
  void CableTargetTick();
  AActor* TargetedOriginPort{nullptr};
  AVCVPort* TargetedDestinationPort{nullptr};
  AActor* TargetedCableEnd{nullptr};
  ACableEnd* HeldCableEnd;

  bool bIsWidgetInteracting{false};

  void NullifyInteractionTargets(AActor* Except = nullptr);

  bool ControllerIsBusy() {
    return bIsWorldInteracting
      || bIsGrabbing
      || bIsParamInteracting
      || bIsPortInteracting;
  }

  TArray<FDelegateHandle> CableTargetedDelegates;
  TArray<FDelegateHandle> CableHeldDelegates;

  public:
    // delegates
    FOnGrabbableTargetedSignature OnGrabbableTargetedDelegate;
    FOnParamTargetedSignature OnParamTargetedDelegate;
    FOnOriginPortTargetedSignature OnOriginPortTargetedDelegate;
    FOnCableTargetedSignature OnCableTargetedDelegate;
    FOnCableHeldSignature OnCableHeldDelegate;
    void HandleDestinationPortTargeted(AVCVPort* Port);
    void HandleOwnTargetings(AActor* TargetedActor, EControllerHand Hand);
};
