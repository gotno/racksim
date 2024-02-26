#pragma once

#include "VCVData/VCVLibrary.h"
#include "Utility/GrabbableActor.h"
#include "osc3.h"

#include "Library.generated.h"

class UWidgetComponent;
class ULibraryWidget;
class ULibraryEntryWidget;
class ULibraryEntry;
class UBasicListEntryData;

UCLASS()
class OSC3_API ALibrary : public AGrabbableActor {
	GENERATED_BODY()
	
public:	
	ALibrary();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  void Refresh();
  void RefreshLibraryList();
  void RefreshBrandFilterList();
  void RefreshTagsFilterList();
  void AddFilter(UBasicListEntryData* BasicListEntryData);
  void RemoveFilter(UBasicListEntryData* BasicListEntryData);
  void SetFavoritesFilterActive(bool bActive);
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void ClearBrandFilter();
  void ClearTagsFilter();

  void SetJsonPath(FString& JsonPath);
  void GetModuleLandingPosition(const float& ModuleWidth, FVector& Location, FRotator& Rotation);
private:

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  
  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* LibraryWidgetComponent;
  ULibraryWidget* LibraryWidget;
  
  void ParseLibraryJson(FString& JsonStr);
  VCVLibrary Model;

  TArray<ULibraryEntry*> GenerateLibraryEntries();
  TArray<UBasicListEntryData*> GenerateBrandFilterEntries();
  TArray<UBasicListEntryData*> GenerateTagsFilterEntries();
  FString BrandFilter;
  TSet<int> TagFilters;
  bool bFavoritesFilterActive;
  
  void SetScale();
  float DesiredWidth{14.f * RENDER_SCALE};
  float BasePadding{0.4f};
};