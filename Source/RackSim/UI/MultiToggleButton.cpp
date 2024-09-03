#include "UI/MultiToggleButton.h"
#include "UI/ToggleButtonToggle.h" 

#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMultiToggleButton::SynchronizeProperties() {
  Super::SynchronizeProperties();

  if (Label) {
    SetPrimaryLabel(LabelText.ToString());
  }
}

void UMultiToggleButton::NativeConstruct() {
  Button->OnReleased.AddDynamic(this, &UMultiToggleButton::HandleClick);
  ToggleOne->OnToggledDelegate.AddDynamic(this, &UMultiToggleButton::HandleToggle);
  ToggleTwo->OnToggledDelegate.AddDynamic(this, &UMultiToggleButton::HandleToggle);
}

void UMultiToggleButton::SetPrimaryLabel(const FString& Text) {
  LabelText = FText::FromString(Text);
  Label->SetText(LabelText);
} 

void UMultiToggleButton::SetToggleLabels(const FString& ToggleOneLabel, const FString& ToggleTwoLabel) {
  // by default,
  // only one toggle is visible
  // and its button is disabled
  // and it has no label.
  //
  // SetToggleLabels() will make both visible,
  // and enable their buttons
  // and set their labels.
  ToggleOne->SetLabel(ToggleOneLabel);
  ToggleTwo->SetLabel(ToggleTwoLabel);

  EnableMultiToggle();
}

void UMultiToggleButton::EnableMultiToggle() {
  Type = FMultiToggleType::Multi;

  ToggleOne->Label->SetVisibility(ESlateVisibility::HitTestInvisible);
  ToggleOne->Button->SetVisibility(ESlateVisibility::Visible);

  ToggleTwo->SetVisibility(ESlateVisibility::Visible);
}

void UMultiToggleButton::HandleClick() {
  if (Type == FMultiToggleType::Single) {
    ToggleOne->ToggleChecked();
    OnToggleOneToggledDelegate.Broadcast(ToggleOne->IsChecked());
    return;
  }

  // always all toggled first, then all not
  bool bChecked =
    !ToggleOne->IsChecked() || !ToggleTwo->IsChecked()
      ? true
      : false;

  ToggleOne->SetChecked(bChecked);
  ToggleTwo->SetChecked(bChecked);

  OnToggleOneToggledDelegate.Broadcast(bChecked);
  OnToggleTwoToggledDelegate.Broadcast(bChecked);
}

void UMultiToggleButton::HandleToggle(UToggleButtonToggle* Toggle, bool bIsChecked) {
  if (Toggle == ToggleOne) {
    OnToggleOneToggledDelegate.Broadcast(bIsChecked);
  } else {
    OnToggleTwoToggledDelegate.Broadcast(bIsChecked);
  }
}

void UMultiToggleButton::SetToggleOneChecked(bool bIsChecked) {
  ToggleOne->SetChecked(bIsChecked);
}

void UMultiToggleButton::SetToggleTwoChecked(bool bIsChecked) {
  ToggleTwo->SetChecked(bIsChecked);
}