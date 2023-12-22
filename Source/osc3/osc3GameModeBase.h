#pragma once

#include "CoreMinimal.h"

#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "VCV.h"
#include "VCVLibrary.h"

#include "GameFramework/GameMode.h"
#include "osc3GameModeBase.generated.h"

class AOSCController;
class AAvatar;
class AVRAvatar;
class AVCVCable;
class AVCVModule;
class AVCVPort;
class ALibrary;
class AWidgetSurrogate;
class Aosc3PlayerController;
class UDPSVGAsset;
class UTexture2D;

UCLASS()
class OSC3_API Aosc3GameModeBase : public AGameMode {
	GENERATED_BODY()

public:
    Aosc3GameModeBase();

private:
  virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
  void SpawnModule(VCVModule module);
  void QueueCableSpawn(VCVCable cable);

  void GetPortInfo(
    PortIdentity identity,
    FVector& portLocation,
    FVector& portForwardVector
  );
  AVCVPort* GetPortActor(PortIdentity identity);
  AVCVCable* SpawnCable(VCVCable cable);
  void DestroyCable(int64_t cableId);
  AVCVCable* DetachCable(int64_t cableId, PortIdentity identity);
  void AttachCable(int64_t cableId, PortIdentity identity);

  void UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color);
  void UpdateParamDisplayValue(int64_t moduleId, int32 paramId, FString displayValue);
  void UpdateModuleMenuItem(VCVMenuItem& MenuItem);
  void ModuleMenuSynced(VCVMenu& Menu);

  void SendParamUpdate(int64_t moduleId, int32 paramId, float value);
  void DuplicateModule(AVCVModule* Module);
  void DestroyModule(AVCVModule* Module);
  void RequestModuleSpawn(FString PluginSlug, FString ModuleSlug);
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void RequestMenu(const VCVMenu& Menu) const;
  void ClickMenuItem(const int64_t& ModuleId, const int& MenuId, const int& MenuItemIndex) const;
  
  void RegisterSVG(FString filepath, Vec2 size);
  void RegisterTexture(FString filepath, UTexture2D* texture);
  UTexture2D* GetTexture(FString filepath);
  
  void SpawnLibrary();
  void UpdateLibrary(VCVLibrary& library);
  ALibrary* GetLibrary();
private:
  UPROPERTY()
  AOSCController* OSCctrl;

  Aosc3PlayerController* PlayerController;
  AVRAvatar* PlayerPawn;

  void ProcessSpawnCableQueue();
  TArray<VCVCable> cableQueue;

  UPROPERTY()
  TMap<int64, AVCVModule*> ModuleActors;
  UPROPERTY()
  TMap<int64, AVCVCable*> CableActors;
  UPROPERTY()
  ALibrary* LibraryActor{nullptr};

  FDPSVGImporter SVGImporter;
  UPROPERTY()
  TMap<FString, UDPSVGAsset*> SVGAssets;
  UPROPERTY()
  TMap<FString, AWidgetSurrogate*> SVGWidgetSurrogates;
  UPROPERTY()
  TMap<FString, UTexture2D*> SVGTextures;
};
