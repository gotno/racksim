#include "UI/LabeledSlider.h"

#include "osc3.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Overlay.h"

void ULabeledSlider::SynchronizeProperties() {
  Super::SynchronizeProperties();
  
  SetLabel(LabelText.ToString());
  SetShowAdjusters(false);
  SetStepsFromStepSize();

  if (Slider) {
    Slider->SetValue(Value);
    Slider->SetMinValue(MinValue);
    Slider->SetMaxValue(MaxValue);
    Slider->SetStepSize(StepSize);
  }
}

void ULabeledSlider::NativeConstruct() {
  Slider->OnValueChanged.AddDynamic(this, &ULabeledSlider::HandleValueChanged);
  Slider->OnMouseCaptureEnd.AddDynamic(this, &ULabeledSlider::HandleMouseCaptureEnd);

  DownCoarseButton->OnClicked.AddDynamic(this, &ULabeledSlider::HandleDownCoarseButtonClicked);
  DownFineButton->OnClicked.AddDynamic(this, &ULabeledSlider::HandleDownFineButtonClicked);
  UpFineButton->OnClicked.AddDynamic(this, &ULabeledSlider::HandleUpFineButtonClicked);
  UpCoarseButton->OnClicked.AddDynamic(this, &ULabeledSlider::HandleUpCoarseButtonClicked);
}

void ULabeledSlider::SetValue(float inValue) {
  float clamped = FMath::Clamp(inValue, MinValue, MaxValue);
  Value = bExponential ? ExponentialToLinear(clamped) : clamped;
  if (Slider) Slider->SetValue(Value);
  UpdateLabel();
}

float ULabeledSlider::GetValue() {
  if (bExponential) return ExponentialFromLinear();
  return Slider->GetValue();
}

void ULabeledSlider::SetExponential(bool inbExponential) {
  bExponential = inbExponential;
  UpdateLabel();
}

void ULabeledSlider::SetLabelPrecision(int inLabelDecimalPlaces) {
  LabelDecimalPlaces = inLabelDecimalPlaces;
  UpdateLabel();
}

void ULabeledSlider::SetShowCoarseAdjusters(bool bShow) {
  if (DownCoarseContainer)
    DownCoarseContainer->SetVisibility(
      bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
  if (UpCoarseContainer)
    UpCoarseContainer->SetVisibility(
      bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void ULabeledSlider::SetShowFineAdjusters(bool bShow) {
  if (DownFineContainer)
    DownFineContainer->SetVisibility(
      bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
  if (UpFineContainer)
    UpFineContainer->SetVisibility(
      bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void ULabeledSlider::SetShowAdjusters(bool bShow) {
  SetShowCoarseAdjusters(bShow);
  SetShowFineAdjusters(bShow);
}

float ULabeledSlider::ExponentialFromLinear() {
  // convert slider value to 0-1 range
  float normalizedSliderPosition = (Value - MinValue) / (MaxValue - MinValue);

  // convert normalized value to exponential scale
  float expScale = (pow(20, normalizedSliderPosition) - 1) / 19;

  // get exponential value
  float expValue = MinValue + (MaxValue - MinValue) * expScale;

  // round to nearest StepSize
  return FMath::RoundToFloat(expValue / StepSize) * StepSize;
}

float ULabeledSlider::ExponentialToLinear(float inValue) {
  // convert slider value to 0-1 range
  float normalizedSliderPosition = (inValue - MinValue) / (MaxValue - MinValue);

  // reverse the exponential scaling
  float linearScale = log10(normalizedSliderPosition * 19 + 1) / log10(20);

  // get linear value
  float linearValue = MinValue + (MaxValue - MinValue) * linearScale;

  // round to nearest StepSize
  return FMath::RoundToFloat(linearValue / StepSize) * StepSize;
}

void ULabeledSlider::SetLabel(const FString& Text) {
  LabelText = FText::FromString(Text);
  UpdateLabel();
}

void ULabeledSlider::SetMinValue(float inMinValue) {
  MinValue = inMinValue;
  if (Slider) Slider->SetMinValue(MinValue);
  UpdateLabel();
}

void ULabeledSlider::SetMaxValue(float inMaxValue) {
  MaxValue = inMaxValue;
  if (Slider) Slider->SetMaxValue(MaxValue);
  UpdateLabel();
}

void ULabeledSlider::SetStepSize(float inStepSize) {
  StepSize = inStepSize;
  if (Slider) Slider->SetStepSize(StepSize);
  SetStepsFromStepSize();
  UpdateLabel();
}

void ULabeledSlider::OverrideCoarseStep(float inCoarseStep) {
  CoarseStep = inCoarseStep;
}

void ULabeledSlider::OverrideFineStep(float inFineStep) {
  FineStep = inFineStep;
}

void ULabeledSlider::SetUnit(const FString& inUnit) {
  Unit = inUnit;
  UpdateLabel();
}

void ULabeledSlider::UpdateLabel() {
  if (!Label) return;

  FString newLabel = LabelText.ToString();
  switch (LabelDecimalPlaces) {
    case 0:
      newLabel = newLabel.Appendf(TEXT(" %.0f%s"), GetValue(), *Unit);
      break;
    case 1:
      newLabel = newLabel.Appendf(TEXT(" %.1f%s"), GetValue(), *Unit);
      break;
    case 2:
      newLabel = newLabel.Appendf(TEXT(" %.2f%s"), GetValue(), *Unit);
      break;
    case 3:
      newLabel = newLabel.Appendf(TEXT(" %.3f%s"), GetValue(), *Unit);
      break;
    default: // anything > 3 gets 4 decimal places.
      newLabel = newLabel.Appendf(TEXT(" %.4f%s"), GetValue(), *Unit);
      break;
  }
  Label->SetText(FText::FromString(newLabel));
}

void ULabeledSlider::SetStepsFromStepSize() {
  FineStep = StepSize;
  CoarseStep = StepSize * 10;
}

void ULabeledSlider::HandleValueChanged(float NewValue) {
  Value = NewValue;
  UpdateLabel();
  OnValueChanged.Broadcast(GetValue());
}

void ULabeledSlider::HandleMouseCaptureEnd() {
  OnMouseCaptureEnd.Broadcast();
}

void ULabeledSlider::Broadcast() {
  OnMouseCaptureEnd.Broadcast();
  OnValueChanged.Broadcast(GetValue());
}

void ULabeledSlider::HandleDownCoarseButtonClicked() {
  SetValue(GetValue() - CoarseStep);
  Broadcast();
}

void ULabeledSlider::HandleDownFineButtonClicked() {
  SetValue(GetValue() - FineStep);
  Broadcast();
}

void ULabeledSlider::HandleUpFineButtonClicked() {
  SetValue(GetValue() + FineStep);
  Broadcast();
}

void ULabeledSlider::HandleUpCoarseButtonClicked() {
  SetValue(GetValue() + CoarseStep);
  Broadcast();
}