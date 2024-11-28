#pragma once

#include "VCVData/VCVLibrary.h"
#include "Utility/GrabbableActor.h"
#include "osc3.h"

#include "Library.generated.h"

class Aosc3GameModeBase;
class AVCVModule;

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

  void SetPreviewsPath(FString& Path);
  void ShowPreview(FString& PluginSlug, FString& ModuleSlug);
  void HidePreview();

  void SetJsonPath(FString& JsonPath);
  void GetPosition(FVector& Location, FRotator& Rotation);

  void GetModuleLandingPosition(const float& ModuleWidth, FVector& Location, FRotator& Rotation);
  void ParkModule(AVCVModule* Module);
private:
  Aosc3GameModeBase* GameMode;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* PreviewMeshComponent;
  UPROPERTY()
  UMaterialInstanceDynamic* PreviewMaterialInstance;
  UPROPERTY()
  UMaterialInterface* PreviewMaterialInterface;
  UPROPERTY()
  UTexture2D* PreviewTexture;
  UPROPERTY()
  UMaterialInstanceDynamic* LoadingMaterialInstance;
  UPROPERTY()
  UMaterialInterface* LoadingMaterialInterface;

  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* LibraryWidgetComponent;
  ULibraryWidget* LibraryWidget;

  void ParseLibraryJson(FString& JsonStr);
  VCVLibrary Model;

  FString PreviewsPath;
  FString PreviewVersionsJsonPath;
  void ParsePreviewVersionsJson();
  void CheckPluginVersionsForPreviewUpdates();
  bool bCheckedPreviews{false};

  TArray<ULibraryEntry*> GenerateLibraryEntries();
  TArray<UFilterListEntryData*> GenerateBrandFilterEntries();
  TArray<UFilterListEntryData*> GenerateTagsFilterEntries();
  FString BrandFilter;
  TSet<int> TagFilters;
  bool bFavoritesFilterActive;

  void SetScale();
  float DesiredWidth{14.f * DEFAULT_RENDER_SCALE};
  float BasePadding{0.4f};

  UPROPERTY()
  TArray<AVCVModule*> ParkedModules;
  UFUNCTION()
  void UnparkModule(AVCVModule* Module);
  // arrange parked modules
  void UpdateLot(float Offset = 0.f, bool bInstant = false);
  void UpdateLot(bool bInstant);
  float ParkingLotGutter{1.f};
};