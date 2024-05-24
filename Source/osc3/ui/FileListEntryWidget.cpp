#include "UI/FileListEntryWidget.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "UI/FileListEntryData.h"

#include "CommonTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

#include "Kismet/GameplayStatics.h"

void UFileListEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  Button->OnHovered.AddDynamic(this, &UFileListEntryWidget::HandleHover);
  Button->OnUnhovered.AddDynamic(this, &UFileListEntryWidget::HandleUnhover);
  Button->OnReleased.AddDynamic(this, &UFileListEntryWidget::HandleClick);
}

void UFileListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
  EntryData = Cast<UFileListEntryData>(ListItemObject);
  Label->SetText(FText::FromString(EntryData->Path));
  LabelStill->SetText(FText::FromString(EntryData->Label));
  Button->SetIsEnabled(!EntryData->Path.Equals(""));

  FolderIcon->SetVisibility(
    EntryData->Icon == EFileIcon::Directory
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  FileIcon->SetVisibility(
    EntryData->Icon == EFileIcon::File
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
}

void UFileListEntryWidget::HandleClick() {
  EntryData->ClickCallback(EntryData->Path);
}

void UFileListEntryWidget::HandleHover() {
  if (!EntryData->bScrollHover) return;

  Label->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
  Label->ResetScrollState();
  LabelStill->SetVisibility(ESlateVisibility::Hidden);
}

void UFileListEntryWidget::HandleUnhover() {
  if (!EntryData->bScrollHover) return;

  Label->SetVisibility(ESlateVisibility::Hidden);
  LabelStill->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}