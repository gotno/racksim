#include "ui/ContextMenuEntry.h"

#include "osc3GameModeBase.h"
#include "ui/ContextMenuEntryData.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UContextMenuEntry::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void UContextMenuEntry::NativeOnListItemObjectSet(UObject* ListItemObject) {
	UContextMenuEntryData* entry = Cast<UContextMenuEntryData>(ListItemObject);
  VCVMenuItem& menuItem = entry->MenuItem;
  
  ActionContainer->SetVisibility(ESlateVisibility::Hidden);
  SubmenuIndicator->SetVisibility(ESlateVisibility::Hidden);
  SelectedIndicator->SetVisibility(ESlateVisibility::Hidden);

  LabelContainer->SetVisibility(ESlateVisibility::Hidden);

  Range->SetVisibility(ESlateVisibility::Hidden);
  
  UBorder* Container{nullptr};
  
  switch (menuItem.type) {
    case VCVMenuItemType::ACTION:
      Container = ActionContainer;
      ActionButtonText->SetText(FText::FromString(menuItem.text));
      if (menuItem.checked) SelectedIndicator->SetVisibility(ESlateVisibility::Visible);
      break;
    case VCVMenuItemType::SUBMENU:
      Container = ActionContainer;
      ActionButtonText->SetText(FText::FromString(menuItem.text));
      SubmenuIndicator->SetVisibility(ESlateVisibility::Visible);
      break;
    case VCVMenuItemType::LABEL:
      Container = LabelContainer;
      LabelText->SetText(FText::FromString(menuItem.text));
      break;
    case VCVMenuItemType::RANGE:
      Container = Range;
      // Range->SetVisibility(ESlateVisibility::Visible);
      break;
    default:
      break;
  }

  // TEMP
  if (!Container) return;

  Container->SetVisibility(ESlateVisibility::Visible);
  Container->SetPadding(
    FMargin(
      0.f,
      entry->dividerPrev ? 2.f : 0.f,
      0.f,
      entry->dividerNext ? 2.f : 0.f
    )
  );
}