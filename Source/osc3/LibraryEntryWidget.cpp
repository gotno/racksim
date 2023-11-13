#include "LibraryEntryWidget.h"

#include "osc3GameModeBase.h"
#include "LibraryEntry.h"

#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"

void ULibraryEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  Button->OnReleased.AddDynamic(this, &ULibraryEntryWidget::RequestModuleSpawn);
  FavoriteToggleButton->OnHovered.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteHover);
  FavoriteToggleButton->OnUnhovered.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteUnhover);
  FavoriteToggleButton->OnReleased.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteClick);
}

void ULibraryEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	ULibraryEntry* entry = Cast<ULibraryEntry>(ListItemObject);

	PluginName->SetText(FText::FromString(entry->PluginName));
  ModuleName->SetText(FText::FromString(entry->ModuleName));
  Tags->SetText(FText::FromString(entry->Tags));

  PluginSlug = entry->PluginSlug;
  ModuleSlug = entry->ModuleSlug;
  bFavorite = entry->bFavorite;

  FavoriteToggleButtonLabel->SetText(
    bFavorite
      ? FText::FromString(FString(TEXT("★")))
      : FText::FromString(FString(TEXT("☆")))
  );
}

void ULibraryEntryWidget::RequestModuleSpawn() {
  // UE_LOG(LogTemp, Warning, TEXT("requesting spawn- %s:%s"), *PluginSlug, *ModuleSlug);
  if (GameMode) GameMode->RequestModuleSpawn(PluginSlug, ModuleSlug);
}

void ULibraryEntryWidget::HandleFavoriteClick() {
  GameMode->SetModuleFavorite(PluginSlug, ModuleSlug, !bFavorite);
}

void ULibraryEntryWidget::HandleFavoriteHover() {
  FavoriteToggleButtonLabel->SetColorAndOpacity(HoverColor);
}

void ULibraryEntryWidget::HandleFavoriteUnhover() {
  FavoriteToggleButtonLabel->SetColorAndOpacity(DefaultColor);
}