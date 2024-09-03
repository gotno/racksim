#include "UI/LibraryWidget.h"

#include "osc3GameModeBase.h"
#include "Library.h"
#include "UI/LibraryEntry.h"
#include "UI/FilterListEntryData.h"

#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/Border.h"

#include "Kismet/GameplayStatics.h"

void ULibraryWidget::NativeConstruct() {
	Super::NativeConstruct();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  
  LibraryOverlayButton->OnReleased.AddDynamic(this, &ULibraryWidget::CollapseAll);
  BrandFilterToggleButton->OnReleased.AddDynamic(this, &ULibraryWidget::ToggleBrandFilterList);
  BrandFilterClearButton->OnReleased.AddDynamic(this, &ULibraryWidget::ClearBrandFilter);
  TagsFilterToggleButton->OnReleased.AddDynamic(this, &ULibraryWidget::ToggleTagsFilterList);
  TagsFilterClearButton->OnReleased.AddDynamic(this, &ULibraryWidget::ClearTagsFilter);
  FavoritesFilterToggleButton->OnReleased.AddDynamic(this, &ULibraryWidget::ToggleFavoritesFilter);
}

void ULibraryWidget::SetLibraryListItems(TArray<ULibraryEntry*> Entries) {
  LibraryListView->SetListItems(Entries);
  LibraryListView->SetScrollbarVisibility(ESlateVisibility::Visible);
}

void ULibraryWidget::SetBrandFilterListItems(TArray<UFilterListEntryData*> Entries) {
  BrandFilterListView->SetListItems(Entries);
}

void ULibraryWidget::SetTagsFilterListItems(TArray<UFilterListEntryData*> Entries) {
  TagsFilterListView->SetListItems(Entries);
}

void ULibraryWidget::SetBrandFilterButtonLabel(const FString& inLabel, const bool& bClearable) const {
  BrandFilterClearButtonContainer->SetVisibility(bClearable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
  BrandFilterToggleButtonLabel->SetText(FText::FromString(inLabel));
  // center justified text doesn't honor the overflow:ellipsis policy
  BrandFilterToggleButtonLabel->SetJustification(bClearable ? ETextJustify::Left : ETextJustify::Center);
}

void ULibraryWidget::SetTagsFilterButtonLabel(const FString& inLabel, const bool& bClearable) const {
  TagsFilterClearButtonContainer->SetVisibility(bClearable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
  TagsFilterToggleButtonLabel->SetText(FText::FromString(inLabel));
  // center justified text doesn't honor the overflow:ellipsis policy
  TagsFilterToggleButtonLabel->SetJustification(bClearable ? ETextJustify::Left : ETextJustify::Center);
}

void ULibraryWidget::ToggleBrandFilterList() {
  SetBrandFilterListVisible(BrandFilterContainer->GetVisibility() != ESlateVisibility::Visible);
}

void ULibraryWidget::SetBrandFilterListVisible(bool bVisible) {
  BrandFilterContainer->SetVisibility(
    bVisible
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  if (bVisible) {
    TagsFilterContainer->SetVisibility(ESlateVisibility::Collapsed);
    LibraryOverlayButton->SetVisibility(ESlateVisibility::Visible);
  } else {
    LibraryOverlayButton->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

void ULibraryWidget::ToggleTagsFilterList() {
  SetTagsFilterListVisible(TagsFilterContainer->GetVisibility() != ESlateVisibility::Visible);
}

void ULibraryWidget::SetTagsFilterListVisible(bool bVisible) {
  TagsFilterContainer->SetVisibility(
    bVisible
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  if (bVisible) {
    BrandFilterContainer->SetVisibility(ESlateVisibility::Collapsed);
    LibraryOverlayButton->SetVisibility(ESlateVisibility::Visible);
  } else {
    LibraryOverlayButton->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

void ULibraryWidget::ClearBrandFilter() {
  GameMode->GetLibrary()->ClearBrandFilter();
}

void ULibraryWidget::ClearTagsFilter() {
  GameMode->GetLibrary()->ClearTagsFilter();
}

void ULibraryWidget::ToggleFavoritesFilter() {
  FavoritesFilterActive = !FavoritesFilterActive;
  
  FavoritesFilterToggleButtonLabel->SetText(
    FavoritesFilterActive
      ? FText::FromString(FString(TEXT("★")))
      : FText::FromString(FString(TEXT("☆")))
  );
  GameMode->GetLibrary()->SetFavoritesFilterActive(FavoritesFilterActive);
}

void ULibraryWidget::CollapseAll() {
  TagsFilterContainer->SetVisibility(ESlateVisibility::Collapsed);
  BrandFilterContainer->SetVisibility(ESlateVisibility::Collapsed);
  LibraryOverlayButton->SetVisibility(ESlateVisibility::HitTestInvisible);
}