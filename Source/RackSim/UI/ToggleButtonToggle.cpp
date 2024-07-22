#include "UI/ToggleButtonToggle.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UToggleButtonToggle::NativeConstruct() {
  Button->OnReleased.AddDynamic(this, &UToggleButtonToggle::HandleClick);
}

void UToggleButtonToggle::SynchronizeProperties() {
  Super::SynchronizeProperties();

  if (Label && Button ) {
    SetLabel(LabelText.ToString());
  }

  if (IconChecked && IconUnchecked) {
    SetChecked(bChecked);
  }
}

void UToggleButtonToggle::SetLabel(const FString& Text) {
  LabelText = FText::FromString(Text);
  Label->SetText(LabelText);

  Label->SetVisibility(
    LabelText.IsEmpty()
      ? ESlateVisibility::Collapsed
      : ESlateVisibility::SelfHitTestInvisible
  );

  Button->SetVisibility(
    LabelText.IsEmpty()
    ? ESlateVisibility::Collapsed
    : ESlateVisibility::Visible
  );
}

void UToggleButtonToggle::ToggleChecked() {
  SetChecked(!bChecked);
}

void UToggleButtonToggle::SetChecked(bool inbChecked) {
  bChecked = inbChecked;

  IconChecked->SetVisibility(
    bChecked
      ? ESlateVisibility::SelfHitTestInvisible
      : ESlateVisibility::Collapsed
  );
  IconUnchecked->SetVisibility(
    bChecked
      ? ESlateVisibility::Collapsed
      : ESlateVisibility::SelfHitTestInvisible
  );
}

void UToggleButtonToggle::HandleClick() {
  ToggleChecked();
  OnToggledDelegate.Broadcast(this, bChecked);
}
