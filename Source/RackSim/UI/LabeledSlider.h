#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LabeledSlider.generated.h"

class UTextBlock;
class USlider;

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

  void SetLabel(const FString& Text);
  void SetLabelPrecision(int inDecimalPlaces);
  void SetUnit(const FString& inUnit);

  void SetExponential(bool inbExponential);

protected:
  virtual void NativeConstruct() override;

private:
  void UpdateLabel();

  UPROPERTY(meta = (BindWidget))
  class UTextBlock* Label;
  FText LabelText;
  FString Unit{TEXT("")}; // directly concat'd with Value
  int LabelDecimalPlaces{0};

  UPROPERTY(meta = (BindWidget))
  class USlider* Slider;
  float Value{0.5f};
  float MinValue{0.f}, MaxValue{1.f};
  float StepSize{0.1f};

  bool bExponential{false};

  float ExponentialFromLinear();
  float ExponentialToLinear(float inValue);

  UFUNCTION()
  void HandleValueChanged(float NewValue);
  UFUNCTION()
  void HandleMouseCaptureEnd();
};
