#include "LibraryWidget.h"

#include "LibraryEntry.h"

#include "Components/ListView.h"

void ULibraryWidget::NativeConstruct() {
	Super::NativeConstruct();
}

void ULibraryWidget::SetListItems(TArray<ULibraryEntry*> Entries) {
  LibraryListView->SetListItems(Entries);
}