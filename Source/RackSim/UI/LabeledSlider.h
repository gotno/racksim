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
  void SetMinValue(float inValue);
  void SetMaxValue(float inValue);
  void SetValueMultiplier(float inMultiplier);
  void SetUnit(const FString& inUnit);
  void SetLabel(const FString& Text);

protected:
  virtual void NativeConstruct() override;

private:
  void UpdateLabel();

  UPROPERTY(meta = (BindWidget))
  class UTextBlock* Label;
  FText LabelText;
  FString Unit{TEXT("%")}; // directly concat'd with Value

  UPROPERTY(meta = (BindWidget))
  class USlider* Slider;
  float Value;
  float MinValue{0.f}, MaxValue{1.f};
  float ValueMultiplier{100.f};

  UFUNCTION()
  void HandleValueChanged(float Value);
  UFUNCTION()
  void HandleMouseCaptureEnd();
};
