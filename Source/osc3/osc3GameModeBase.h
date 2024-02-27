#pragma once

#include "CoreMinimal.h"

#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "VCVData/VCV.h"

#include "GameFramework/GameMode.h"
#include "osc3GameModeBase.generated.h"

class AOSCController;
class URackManager;
class AVRAvatar;
class AVCVCable;
class AVCVModule;
class AContextMenu;
class AVCVPort;
class ALibrary;
class AWidgetSurrogate;
class Aosc3PlayerController;
class UDPSVGAsset;
class UTexture2D;

struct ReturnModulePosition {
  FVector Location{0.f};
  FRotator Rotation{0.f};
  
  // spawned from library is zeros, we'll calculate later
  ReturnModulePosition() {}

  // duplicated from module
  ReturnModulePosition(FVector _Location, FRotator _Rotation)
    : Location(_Location), Rotation(_Rotation) {}
};

UCLASS()
class OSC3_API Aosc3GameModeBase : public AGameMode {
	GENERATED_BODY()

public:
    Aosc3GameModeBase();

private:
  virtual void Tick(float DeltaTime) override;

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  void SpawnModule(VCVModule vcv_module);
  void QueueCableSpawn(VCVCable vcv_cable);
  void RequestExit();

  AVCVCable* SpawnCable(AVCVPort* Port);
  void SpawnCable(int64_t& Id, AVCVPort* InputPort, AVCVPort* OutputPort);
  void DestroyCableActor(AVCVCable* Cable);
  void RegisterCableConnect(AVCVPort* InputPort, AVCVPort* OutputPort);
  void RegisterCableDisconnect(AVCVCable* Cable);

  void UpdateLight(int64_t ModuleId, int32 LightId, FLinearColor Color);
  void UpdateParam(int64_t ModuleId, VCVParam& Param);

  void SendParamUpdate(int64_t ModuleId, int32 ParamId, float Value);
  void DuplicateModule(AVCVModule* Module);
  void DestroyModule(int64_t ModuleId, bool bSync = true);
  void RequestModuleSpawn(FString PluginSlug, FString ModuleSlug);
  void RequestModuleDiff(const int64_t& ModuleId) const;
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void RequestMenu(const FVCVMenu& Menu) const;
  void ClickMenuItem(const FVCVMenuItem& MenuItem);
  void UpdateMenuItemQuantity(const FVCVMenuItem& MenuItem, const float& Value) const;
  
  void RegisterSVG(FString Filepath, Vec2 Size);
  void RegisterTexture(FString Filepath, UTexture2D* Texture);
  UTexture2D* GetTexture(FString Filepath);
  
  void SpawnLibrary();
  void SetLibraryJsonPath(FString& Path);
  ALibrary* GetLibrary();

private:
  UFUNCTION()
  void Exit();

  UPROPERTY()
  URackManager* rackman;
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

  int64_t LastClickedMenuModuleId{-1};
  int currentReturnModuleId{0};
  TMap<int32, ReturnModulePosition> ReturnModulePositions;

  FDPSVGImporter SVGImporter;
  UPROPERTY()
  TMap<FString, UDPSVGAsset*> SVGAssets;
  UPROPERTY()
  TMap<FString, AWidgetSurrogate*> SVGWidgetSurrogates;
  UPROPERTY()
  TMap<FString, UTexture2D*> SVGTextures;

public:
  // delegate stuff
  void SubscribeMenuItemSyncedDelegate(AContextMenu* ContextMenu);
  void SubscribeMenuSyncedDelegate(AContextMenu* ContextMenu);
};
