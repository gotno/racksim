#pragma once

#include "osc3.h"
#include "VCV.h"
#include "Grabbable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVModule.generated.h"

class UTexture2D;
class Aosc3GameModeBase;
class AVCVLight;
class AVCVPort;
class AVCVParam;
class UContextMenu;
class UWidgetComponent;

UCLASS()
class OSC3_API AVCVModule : public AActor, public IGrabbable {
	GENERATED_BODY()
	
public:	
	AVCVModule();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

  void init(VCVModule model);
  
  int64 GetId();
  void GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector);
  void AttachCable(const PortIdentity& identity, int64_t cableId);
  void DetachCable(const PortIdentity& identity, int64_t cableId);
  void UpdateLight(int32 lightId, FLinearColor color);
  AVCVParam* GetParamActor(const int& paramId) { return ParamActors[paramId]; }
  AVCVPort* GetPortActor(PortIdentity identity);
  void GetSlugs(FString& PluginSlug, FString& Slug);

  void ToggleContextMenu();
  void AddMenuItem(VCVMenuItem& MenuItem);
  void MenuSynced(VCVMenu& Menu);
  void MakeMenu(int ParentMenuId = -1, int ParentItemIndex = -1);
  void RequestMenu(int MenuId);
  void CloseMenu();

  UPROPERTY()
  TMap<int32, AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, AVCVLight*> ParamLightActors;
  
  void registerParamLight(int64_t lightId, AVCVLight* lightActor);
  void paramUpdated(int32 paramId, float value);

  FString Brand;
  FString Name;

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

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* OutlineMaterialInstance;
  UPROPERTY()
  UMaterialInterface* OutlineMaterialInterface;

  UPROPERTY()
  UTexture2D* texture;
  
  UWidgetComponent* ContextMenuWidgetComponent;
  UContextMenu* ContextMenuWidget;
  void SetupContextMenuWidget();
  void SetMenu(int MenuId);
  FString MakeMenuBreadcrumbs(int MenuId);

  void spawnComponents();

  VCVModule model;
  
  Aosc3GameModeBase* gameMode;

  TMap<int, AVCVParam*> ParamActors;
  TMap<int, AVCVPort*> InputActors;
  TMap<int, AVCVPort*> OutputActors;
  
  TArray<VCVMenu> ContextMenus;
};