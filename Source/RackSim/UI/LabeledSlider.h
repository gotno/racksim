#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LabeledSlider.generated.h"

class UTextBlock;
class USlider;
class UButton;
class UOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMouseCaptureEndSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueChangedSignature, float, Value);

UCLASS()
class RACKSIM_API ULabeledSlider : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void SynchronizeProperties() override;

  FOnValueChangedSignature OnValueChanged;
  FOnMouseCaptureEndSignature OnMouseCaptureEnd;

  float GetValue();

  void SetValue(float inValue);
  void SetMinValue(float inMinValue);
  void SetMaxValue(float inMaxValue);
  void SetStepSize(float inStepSize);
  void OverrideCoarseStep(float inCoarseStep);
  void OverrideFineStep(float inFineStep);

  void SetLabel(const FString& Text);
  void SetLabelPrecision(int inDecimalPlaces);
  void SetUnit(const FString& inUnit);

  void SetExponential(bool inbExponential);

  void SetShowCoarseAdjusters(bool bShow);
  void SetShowFineAdjusters(bool bShow);
  void SetShowAdjusters(bool bShow);

protected:
  virtual void NativeConstruct() override;

private:
  void UpdateLabel();

  UPROPERTY(meta = (BindWidget))
  UTextBlock* Label;
  FText LabelText;
  FString Unit{TEXT("")}; // directly concat'd with Value
  int LabelDecimalPlaces{0};

  UPROPERTY(meta = (BindWidget))
  USlider* Slider;
  float Value{0.5f};
  float MinValue{0.f}, MaxValue{1.f};
  float StepSize{0.1f};

  UPROPERTY(meta = (BindWidget))
  UOverlay* DownCoarseContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* DownCoarseButton;

  UPROPERTY(meta = (BindWidget))
  UOverlay* DownFineContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* DownFineButton;

  UPROPERTY(meta = (BindWidget))
  UOverlay* UpFineContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* UpFineButton;

  UPROPERTY(meta = (BindWidget))
  UOverlay* UpCoarseContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* UpCoarseButton;

  float FineStep{0.f};
  float CoarseStep{0.f};
  void SetStepsFromStepSize();

  bool bExponential{false};

  float ExponentialFromLinear();
  float ExponentialToLinear(float inValue);

  UFUNCTION()
  void HandleValueChanged(float NewValue);
  UFUNCTION()
  void HandleMouseCaptureEnd();

  UFUNCTION()
  void HandleDownCoarseButtonClicked();
  UFUNCTION()
  void HandleDownFineButtonClicked();
  UFUNCTION()
  void HandleUpFineButtonClicked();
  UFUNCTION()
  void HandleUpCoarseButtonClicked();

  void Broadcast();
};
