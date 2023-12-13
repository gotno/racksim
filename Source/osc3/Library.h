#pragma once

#include "osc3.h"
#include "VCVLibrary.h"
#include "Grabbable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Library.generated.h"

class UWidgetComponent;
class ULibraryWidget;
class ULibraryEntryWidget;
class ULibraryEntry;
class UBasicListEntryData;

UCLASS()
class OSC3_API ALibrary : public AActor, public IGrabbable {
	GENERATED_BODY()
	
public:	
	ALibrary();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void Update(VCVLibrary& Library);
  
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

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void ReleaseGrab() override;
  void SetHighlighted(bool bHighlighted, FLinearColor OutlineColor = OUTLINE_COLOR) override;
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* OutlineMeshComponent;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* PreviewPlaneComponent;
  UPROPERTY()
  UMaterialInstanceDynamic* PreviewMaterialInstance;
  UPROPERTY()
  UMaterialInterface* PreviewMaterialInterface;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* OutlineMaterialInstance;
  UPROPERTY()
  UMaterialInterface* OutlineMaterialInterface;
  
  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* LibraryWidgetComponent;
  ULibraryWidget* LibraryWidget;
  
  TArray<ULibraryEntry*> GenerateLibraryEntries();
  TArray<UBasicListEntryData*> GenerateBrandFilterEntries();
  TArray<UBasicListEntryData*> GenerateTagsFilterEntries();
  FString BrandFilter;
  TSet<int> TagFilters;
  bool bFavoritesFilterActive;
  
  void SetScale();
  float DesiredWidth{28.f};
  float BasePadding{0.4f};
  
  VCVLibrary Model;
};