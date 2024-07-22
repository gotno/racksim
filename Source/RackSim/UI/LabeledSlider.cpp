#include "UI/LabeledSlider.h"

#include "osc3.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"

void ULabeledSlider::SynchronizeProperties() {
  Super::SynchronizeProperties();
  
  SetLabel(LabelText.ToString());
}

void ULabeledSlider::NativeConstruct() {
  Slider->OnValueChanged.AddDynamic(this, &ULabeledSlider::HandleValueChanged);
  Slider->OnMouseCaptureEnd.AddDynamic(this, &ULabeledSlider::HandleMouseCaptureEnd);
}

float ULabeledSlider::GetValue() {
  return Slider->GetValue();
}

void ULabeledSlider::SetLabel(const FString& Text) {
  LabelText = FText::FromString(Text);
  UpdateLabel();
}

void ULabeledSlider::SetValue(float inValue) {
  Value = inValue;
  if (Slider) Slider->SetValue(inValue);
  UpdateLabel();
}

void ULabeledSlider::SetMinValue(float inValue) {
  MinValue = inValue;
  UpdateLabel();
}

void ULabeledSlider::SetMaxValue(float inValue) {
  MaxValue = inValue;
  UpdateLabel();
}

void ULabeledSlider::SetValueMultiplier(float inMultiplier) {
  ValueMultiplier = inMultiplier;
  UpdateLabel();
}

void ULabeledSlider::SetUnit(const FString& inUnit) {
  Unit = inUnit;
  UpdateLabel();
}

void ULabeledSlider::UpdateLabel() {
  if (!Label) return;

  FString newLabel = LabelText.ToString();
  newLabel = newLabel.Appendf(
    TEXT(" %.0f%s"),
    (Value * (MaxValue - MinValue) + MinValue) * ValueMultiplier,
    *Unit
  );

  Label->SetText(FText::FromString(newLabel));
}

void ULabeledSlider::HandleValueChanged(float inValue) {
  Value = inValue;
  UpdateLabel();
  OnValueChanged.Broadcast(Value);
}

void ULabeledSlider::HandleMouseCaptureEnd() {
  OnMouseCaptureEnd.Broadcast();
}