#include "ui/ContextMenu.h"

#include "ui/ContextMenuEntryData.h"

#include "Components/ListView.h"

void UContextMenu::SetListItems(TArray<UContextMenuEntryData*> Entries) {
  ListView->SetListItems(Entries);
  ListView->SetScrollbarVisibility(ESlateVisibility::Visible);
}