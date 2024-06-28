#pragma once

#include "VCVData/VCVLibrary.h"
#include "Utility/GrabbableActor.h"
#include "osc3.h"

#include "Library.generated.h"

class UWidgetComponent;
class ULibraryWidget;
class ULibraryEntryWidget;
class ULibraryEntry;
class UFilterListEntryData;

UCLASS()
class RACKSIM_API ALibrary : public AGrabbableActor {
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
  void AddFilter(UFilterListEntryData* FilterListEntryData);
  void RemoveFilter(UFilterListEntryData* FilterListEntryData);
  void SetFavoritesFilterActive(bool bActive);
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void ClearBrandFilter();
  void ClearTagsFilter();
  void Summon(FVector Location, FRotator Rotation);

  void SetJsonPath(FString& JsonPath);
  void GetPosition(FVector& Location, FRotator& Rotation);
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
  TArray<UFilterListEntryData*> GenerateBrandFilterEntries();
  TArray<UFilterListEntryData*> GenerateTagsFilterEntries();
  FString BrandFilter;
  TSet<int> TagFilters;
  bool bFavoritesFilterActive;
  
  void SetScale();
  float DesiredWidth{14.f * RENDER_SCALE};
  float BasePadding{0.4f};

  // summoning
  FVector StartLocation, TargetLocation;
  FRotator StartRotation, TargetRotation;
  float SummonAlpha{0.f};
  bool bSummoned;
};