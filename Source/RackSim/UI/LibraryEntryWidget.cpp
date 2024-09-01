#include "UI/LibraryEntryWidget.h"

#include "osc3GameModeBase.h"
#include "Library.h"
#include "UI/LibraryEntry.h"

#include "CommonTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"

void ULibraryEntryWidget::NativeConstruct() {
  Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  Button->OnReleased.AddDynamic(this, &ULibraryEntryWidget::RequestModuleSpawn);
  Button->OnHovered.AddDynamic(this, &ULibraryEntryWidget::HandleButtonHover);
  Button->OnUnhovered.AddDynamic(this, &ULibraryEntryWidget::HandleButtonUnhover);
  FavoriteToggleButton->OnHovered.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteHover);
  FavoriteToggleButton->OnUnhovered.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteUnhover);
  FavoriteToggleButton->OnReleased.AddDynamic(this, &ULibraryEntryWidget::HandleFavoriteClick);
}

void ULibraryEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject) {
	ULibraryEntry* entry = Cast<ULibraryEntry>(ListItemObject);

	PluginName->SetText(FText::FromString(entry->PluginName));
  ModuleName->SetText(FText::FromString(entry->ModuleName));
  TagsText->SetText(FText::FromString(entry->Tags));
  DescriptionText->SetText(FText::FromString(entry->ModuleDescription));

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
  if (!GameMode) return;
  GameMode->GetLibrary()->HidePreview();
  GameMode->RequestModuleSpawn(PluginSlug, ModuleSlug);
}

void ULibraryEntryWidget::HandleButtonHover() {
  if (GameMode) GameMode->GetLibrary()->ShowPreview(PluginSlug, ModuleSlug);
  DescriptionText->SetVisibility(ESlateVisibility::Collapsed);
  TagsText->SetVisibility(ESlateVisibility::Visible);
}

void ULibraryEntryWidget::HandleButtonUnhover() {
  if (GameMode) GameMode->GetLibrary()->HidePreview();
  DescriptionText->SetVisibility(ESlateVisibility::Visible);
  TagsText->SetVisibility(ESlateVisibility::Collapsed);
}

void ULibraryEntryWidget::HandleFavoriteClick() {
  if (GameMode) GameMode->SetModuleFavorite(PluginSlug, ModuleSlug, !bFavorite);
}

void ULibraryEntryWidget::HandleFavoriteHover() {
  FavoriteToggleButtonLabel->SetColorAndOpacity(HoverColor);
}

void ULibraryEntryWidget::HandleFavoriteUnhover() {
  FavoriteToggleButtonLabel->SetColorAndOpacity(DefaultColor);
}