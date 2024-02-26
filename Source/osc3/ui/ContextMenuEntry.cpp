#include "UI/ContextMenuEntry.h"

#include "osc3GameModeBase.h"
#include "VCVModule.h"
#include "UI/ContextMenuEntryData.h"

#include "CommonTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

#include "Kismet/GameplayStatics.h"

void UContextMenuEntry::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
  ActionButton->OnReleased.AddDynamic(this, &UContextMenuEntry::HandleClick);
  ActionButton->OnHovered.AddDynamic(this, &UContextMenuEntry::HandleActionHover);
  ActionButton->OnUnhovered.AddDynamic(this, &UContextMenuEntry::HandleActionUnhover);
  BackButton->OnReleased.AddDynamic(this, &UContextMenuEntry::HandleClick);
  Slider->OnValueChanged.AddDynamic(this, &UContextMenuEntry::HandleSliderChange);
  Slider->OnMouseCaptureEnd.AddDynamic(this, &UContextMenuEntry::HandleSliderRelease);
}

void UContextMenuEntry::NativeOnListItemObjectSet(UObject* ListItemObject) {
	UContextMenuEntryData* entry = Cast<UContextMenuEntryData>(ListItemObject);
  MenuItem = entry->MenuItem;
  Module = entry->Module;
  ParentMenuId = entry->ParentMenuId;
  
  ActionContainer->SetVisibility(ESlateVisibility::Hidden);

  // set up offsets to make space for submenu/bool indicators
  UCanvasPanelSlot* actionTextSlot = Cast<UCanvasPanelSlot>(ActionButtonText->Slot);
  UCanvasPanelSlot* actionTextStillSlot = Cast<UCanvasPanelSlot>(ActionButtonTextStill->Slot);
  FMargin actionTextOffsets = actionTextSlot->GetOffsets();
  // set default
  actionTextOffsets.Right = 0.f;
  actionTextSlot->SetOffsets(actionTextOffsets);
  actionTextStillSlot->SetOffsets(actionTextOffsets);
  // set up for later
  actionTextOffsets.Right = ActionIndicatorsMargin;

  SubmenuIndicator->SetVisibility(ESlateVisibility::Collapsed);
  SelectedIndicator->SetVisibility(ESlateVisibility::Collapsed);

  LabelContainer->SetVisibility(ESlateVisibility::Hidden);

  Range->SetVisibility(ESlateVisibility::Hidden);

  BackContainer->SetVisibility(ESlateVisibility::Hidden);
  
  UBorder* Container{nullptr};
  
  switch (MenuItem.type) {
    case VCVMenuItemType::ACTION:
      Container = ActionContainer;
      ActionButtonText->SetText(FText::FromString(MenuItem.text));
      ActionButtonTextStill->SetText(FText::FromString(MenuItem.text));
      ActionButton->SetIsEnabled(!MenuItem.disabled);

      if (MenuItem.checked) {
        SelectedIndicator->SetVisibility(ESlateVisibility::Visible);
        actionTextSlot->SetOffsets(actionTextOffsets);
        actionTextStillSlot->SetOffsets(actionTextOffsets);
      }
      break;
    case VCVMenuItemType::SUBMENU:
      Container = ActionContainer;
      ActionButtonText->SetText(FText::FromString(MenuItem.text));
      ActionButtonTextStill->SetText(FText::FromString(MenuItem.text));
      SubmenuIndicator->SetVisibility(ESlateVisibility::Visible);
      actionTextSlot->SetOffsets(actionTextOffsets);
      actionTextStillSlot->SetOffsets(actionTextOffsets);
      break;
    case VCVMenuItemType::LABEL:
      Container = LabelContainer;
      LabelText->SetText(FText::FromString(MenuItem.text));
      break;
    case VCVMenuItemType::RANGE:
      Container = Range;
      Slider->SetMinValue(MenuItem.quantityMinValue);
      Slider->SetMaxValue(MenuItem.quantityMaxValue);
      Slider->SetValue(MenuItem.quantityValue);
      SliderText->SetText(FText::FromString(MenuItem.text));
      break;
    case VCVMenuItemType::BACK:
      Container = BackContainer;
      BackButtonText->SetText(FText::FromString(MenuItem.text));
      break;
    default:
      break;
  }

  float bottomMargin{entry->DividerNext ? 2.f : 0.f};
  Container->SetPadding(FMargin( 0.f, 0.f, 0.f, bottomMargin));
  Container->SetVisibility(ESlateVisibility::Visible);
}

void UContextMenuEntry::HandleActionHover() {
  ActionButtonText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  ActionButtonText->ResetScrollState();
  ActionButtonTextStill->SetVisibility(ESlateVisibility::Hidden);
}

void UContextMenuEntry::HandleActionUnhover() {
  ActionButtonText->SetVisibility(ESlateVisibility::Hidden);
  ActionButtonTextStill->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UContextMenuEntry::HandleClick() {
  switch (MenuItem.type) {
    case VCVMenuItemType::ACTION:
      GameMode->ClickMenuItem(MenuItem);
      break;
    case VCVMenuItemType::SUBMENU:
      Module->MakeMenu(MenuItem.menuId, MenuItem.index);
      break;
    case VCVMenuItemType::BACK:
      if (ParentMenuId == -1) {
        Module->CloseMenu();
      } else {
        Module->RequestMenu(ParentMenuId);
      }
      break;
    default:
      break;
  }
}

void UContextMenuEntry::HandleSliderChange(float Value) {
  FString label("");
  if (!MenuItem.quantityLabel.IsEmpty()) {
    label.Append(MenuItem.quantityLabel).Append(FString(": "));
  }
  label.Append(FString::SanitizeFloat(Value)).Append(MenuItem.quantityUnit);
  SliderText->SetText(FText::FromString(label));
}

void UContextMenuEntry::HandleSliderRelease() {
  // send quantity update
  GameMode->UpdateMenuItemQuantity(MenuItem, Slider->GetValue());
}