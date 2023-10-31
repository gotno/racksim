#pragma once

#include "VCVLibrary.h"
#include "Grabbable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Library.generated.h"

class UWidgetComponent;
class ULibraryWidget;
class ULibraryEntryWidget;
class ULibraryEntry;

UCLASS()
class OSC3_API ALibrary : public AActor, public IGrabbable {
	GENERATED_BODY()
	
public:	
	ALibrary();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  void Init(VCVLibrary model);

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void ReleaseGrab() override;
  void SetHighlighted(bool bHighlighted) override;

private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;

  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY()
  UStaticMesh* StaticMesh;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  
  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* LibraryWidgetComponent;
  ULibraryWidget* LibraryWidget;
  
  TArray<ULibraryEntry*> GenerateEntries();
  
  void SetScale();
  float DesiredWidth{20.f};
  float BasePadding{0.4f};
  
  VCVLibrary Model;
};