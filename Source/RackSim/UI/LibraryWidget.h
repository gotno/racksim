#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "LibraryWidget.generated.h"

class UListView;
class UButton;
class UBorder;
class UTextBlock;
class USizeBox;

class Aosc3GameModeBase;
class ULibraryEntry;
class UFilterListEntryData;

UCLASS()
class RACKSIM_API ULibraryWidget : public UUserWidget {
	GENERATED_BODY()

public:
  void SetLibraryListItems(TArray<ULibraryEntry*> Entries);
  void SetBrandFilterListItems(TArray<UFilterListEntryData*> Entries);
  void SetTagsFilterListItems(TArray<UFilterListEntryData*> Entries);
  void SetBrandFilterButtonLabel(const FString& inLabel, const bool& bClearable) const;
  void SetTagsFilterButtonLabel(const FString& inLabel, const bool& bClearable) const;

  UFUNCTION()
  void ToggleBrandFilterList();
  void SetBrandFilterListVisible(bool bVisible);
  UFUNCTION()
  void ToggleTagsFilterList();
  void SetTagsFilterListVisible(bool bVisible);
  UFUNCTION()
  void ClearBrandFilter();
  UFUNCTION()
  void ClearTagsFilter();
  UFUNCTION()
  void CollapseAll();
  UFUNCTION()
  void ToggleFavoritesFilter();

protected:
	virtual void NativeConstruct() override;	
  
  UPROPERTY(meta = (BindWidget))
	UBorder* LibraryContainer;
  UPROPERTY(meta = (BindWidget))
	UListView* LibraryListView;
  UPROPERTY(meta = (BindWidget))
  UButton* LibraryOverlayButton;

  UPROPERTY(meta = (BindWidget))
  UButton* BrandFilterToggleButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* BrandFilterToggleButtonLabel;
  UPROPERTY(meta = (BindWidget))
  USizeBox* BrandFilterClearButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* BrandFilterClearButton;
  UPROPERTY(meta = (BindWidget))
	UBorder* BrandFilterContainer;
  UPROPERTY(meta = (BindWidget))
	UListView* BrandFilterListView;
  
  UPROPERTY(meta = (BindWidget))
  UButton* TagsFilterToggleButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* TagsFilterToggleButtonLabel;
  UPROPERTY(meta = (BindWidget))
  USizeBox* TagsFilterClearButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* TagsFilterClearButton;
  UPROPERTY(meta = (BindWidget))
	UBorder* TagsFilterContainer;
  UPROPERTY(meta = (BindWidget))
	UListView* TagsFilterListView;

  UPROPERTY(meta = (BindWidget))
  UButton* FavoritesFilterToggleButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* FavoritesFilterToggleButtonLabel;
  
private:
  Aosc3GameModeBase* GameMode;
  bool FavoritesFilterActive{false};
};