#include "Tooltip.h"

#include "Components/TextBlock.h"

void UTooltip::SetLabel(FString NewLabel) {
  Label->SetText(FText::FromString(NewLabel));
}

void UTooltip::SetDisplayValue(FString NewDisplayValue) {
  DisplayValue->SetText(FText::FromString(NewDisplayValue));
}