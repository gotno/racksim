#include "UI/ContextMenu.h"

#include "UI/ContextMenuEntryData.h"

#include "Components/ListView.h"

void UContextMenu::SetListItems(TArray<UContextMenuEntryData*> Entries) {
  ListView->SetListItems(Entries);
  ListView->SetScrollbarVisibility(ESlateVisibility::Visible);
}