#include "UI/ContextMenuWidget.h"

#include "UI/ContextMenuEntryData.h"

#include "Components/ListView.h"

void UContextMenuWidget::SetListItems(TArray<UContextMenuEntryData*> Entries) {
  ListView->SetListItems(Entries);
  ListView->SetScrollbarVisibility(ESlateVisibility::Visible);
}