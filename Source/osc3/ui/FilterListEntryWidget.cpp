#include "UI/FilterListEntryWidget.h"

#include "UI/FilterListEntryData.h"
#include "osc3GameModeBase.h"
#include "library.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"

void UFilterListEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  Button->OnReleased.AddDynamic(this, &UFilterListEntryWidget::HandleClick);
}

void UFilterListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	EntryData = Cast<UFilterListEntryData>(ListItemObject);

  Label->SetText(FText::FromString(EntryData->Label));
  SetSelected(EntryData->bSelected);
}

void UFilterListEntryWidget::SetSelected(bool inSelected) {
  bSelected = inSelected;

  SelectedIndicator->SetVisibility(
    bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden
  );
}

void UFilterListEntryWidget::HandleClick() {
  SetSelected(!bSelected);
  
  if (bSelected) {
    GameMode->GetLibrary()->AddFilter(EntryData);
  } else {
    GameMode->GetLibrary()->RemoveFilter(EntryData);
  }
}