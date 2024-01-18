#pragma once

#include "CoreMinimal.h"

#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "VCVData/VCV.h"

#include "GameFramework/GameMode.h"
#include "osc3GameModeBase.generated.h"

class AOSCController;
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

  AVCVCable* SpawnCable(AVCVPort* Port);
  void SpawnCable(int64_t& Id, AVCVPort* InputPort, AVCVPort* OutputPort);
  void DestroyCableActor(AVCVCable* Cable);
  void RegisterCableConnect(AVCVPort* InputPort, AVCVPort* OutputPort);
  void RegisterCableDisconnect(AVCVCable* Cable);

  void UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color);
  void UpdateParam(int64_t moduleId, VCVParam& param);
  void UpdateModuleMenuItem(VCVMenuItem& MenuItem);
  void ModuleMenuSynced(VCVMenu& Menu);

  void SendParamUpdate(int64_t moduleId, int32 paramId, float value);
  void DuplicateModule(AVCVModule* Module);
  void DestroyModule(int64_t ModuleId, bool bSync = true);
  void RequestModuleSpawn(FString PluginSlug, FString ModuleSlug);
  void RequestModuleDiff(const int64_t& ModuleId) const;
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void RequestMenu(const VCVMenu& Menu) const;
  void ClickMenuItem(const VCVMenuItem& MenuItem) const;
  void UpdateMenuItemQuantity(const VCVMenuItem& MenuItem, const float& Value) const;
  
  void RegisterSVG(FString filepath, Vec2 size);
  void RegisterTexture(FString filepath, UTexture2D* texture);
  UTexture2D* GetTexture(FString filepath);
  
  void SpawnLibrary();
  void SetLibraryJsonPath(FString& Path);
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
  TArray<AVCVCable*> CableActors;
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
