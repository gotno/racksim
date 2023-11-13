#include "ui/BasicListEntryWidget.h"

#include "ui/BasicListEntryData.h"
#include "osc3GameModeBase.h"
#include "library.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"

void UBasicListEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  Button->OnReleased.AddDynamic(this, &UBasicListEntryWidget::HandleClick);
}

void UBasicListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	EntryData = Cast<UBasicListEntryData>(ListItemObject);

  Label->SetText(FText::FromString(EntryData->Label));
  SetSelected(EntryData->bSelected);
	// PluginName->SetText(FText::FromString(entry->PluginName));
  // ModuleName->SetText(FText::FromString(entry->ModuleName));
  // Tags->SetText(FText::FromString(entry->Tags));

  // PluginSlug = entry->PluginSlug;
  // ModuleSlug = entry->ModuleSlug;
}

void UBasicListEntryWidget::SetSelected(bool inSelected) {
  bSelected = inSelected;

  SelectedIndicator->SetVisibility(
    bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden
  );
}

void UBasicListEntryWidget::HandleClick() {
  SetSelected(!bSelected);
  
  if (bSelected) {
    GameMode->GetLibrary()->AddFilter(EntryData);
  } else {
    GameMode->GetLibrary()->RemoveFilter(EntryData);
  }
}