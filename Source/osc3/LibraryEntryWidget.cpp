#include "LibraryEntryWidget.h"

#include "LibraryEntry.h"

#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/Button.h"

void ULibraryEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  Button->OnPressed.AddDynamic(this, &ULibraryEntryWidget::PrintSlug);
}

void ULibraryEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	ULibraryEntry* entry = Cast<ULibraryEntry>(ListItemObject);
	PluginName->SetText(FText::FromString(entry->PluginName));
	ModuleName->SetText(FText::FromString(entry->ModuleName));
  PluginSlug = entry->PluginSlug;
  ModuleSlug = entry->ModuleSlug;
  
  // GetOwningListView()->GetOwningPlayer();
}

void ULibraryEntryWidget::PrintSlug() {
  UE_LOG(LogTemp, Warning, TEXT("%s:%s"), *PluginSlug, *ModuleSlug);
}