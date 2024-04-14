#include "UI/FileListEntryWidget.h"

#include "UI/FileListEntryData.h"
#include "osc3GameModeBase.h"
#include "library.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"

void UFileListEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  // GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  Button->OnReleased.AddDynamic(this, &UFileListEntryWidget::HandleClick);
}

void UFileListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	EntryData = Cast<UFileListEntryData>(ListItemObject);
  Label->SetText(FText::FromString(EntryData->Label));
  Button->SetIsEnabled(!EntryData->Path.Equals(""));
}

void UFileListEntryWidget::HandleClick() {
  EntryData->ClickCallback(EntryData->Path);
}