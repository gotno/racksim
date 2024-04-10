#pragma once

#include "CoreMinimal.h"

#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "VCVData/VCV.h"

#include "GameFramework/GameModeBase.h"
#include "osc3GameModeBase.generated.h"

class Aosc3GameState;
class AOSCController;
class AGrabbableActor;
class URackManager;
class AMainMenu;
class Uosc3SaveGame;
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
class AModuleWeldment;

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
class OSC3_API Aosc3GameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
    Aosc3GameModeBase();

private:
  virtual void Tick(float DeltaTime) override;

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  void RackConnectionEstablished();

  void SpawnModule(VCVModule vcv_module);
  void QueueCableSpawn(VCVCable vcv_cable);
  void RequestExit();
  void ToggleMainMenu();
  void SummonLibrary(FVector Location, FRotator Rotation);

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
  void SpawnMainMenu();
  void SetLibraryJsonPath(FString& Path);
  ALibrary* GetLibrary();

  void DestroyWeldment(AModuleWeldment* Weldment);
  void SplitWeldment(AModuleWeldment* Weldment, int AfterIndex);
  void SplitWeldment(AModuleWeldment* Weldment, AVCVModule* OnModule);
  void WeldModules(TArray<int64>& ModuleIds, bool bShouldArrangeRackside = false);
  void WeldModules(AVCVModule* LeftModule, AVCVModule* RightModule, bool bLeftIsAnchor = true, bool bShouldArrangeRackside = true);

private:
  UFUNCTION()
  void Reset();
  UFUNCTION()
  void Exit();

  UFUNCTION()
  Uosc3SaveGame* MakeSaveGame();
  UFUNCTION()
  void Autosave();
  void StartAutosaving();
  void StopAutosaving();
  FTimerHandle hAutosaveTimer;

  UPROPERTY()
  URackManager* rackman;
  UPROPERTY()
  AOSCController* OSCctrl;
  UPROPERTY()
  AMainMenu* MainMenu;

  UPROPERTY()
  Aosc3GameState* osc3GameState;
  UPROPERTY()
  Aosc3PlayerController* PlayerController;
  UPROPERTY()
  AVRAvatar* PlayerPawn;

  FVector DefaultInPatchPlayerLocation{0.f};

  UPROPERTY()
  TMap<int64, AVCVModule*> ModuleActors;

  UPROPERTY()
  TArray<AModuleWeldment*> ModuleWeldments;
  TMap<int64_t, VCVModule> ModulesSeekingWeldment;
  void ProcessWeldmentQueue();

  UPROPERTY()
  TArray<AVCVCable*> CableActors;
  void ProcessSpawnCableQueue();
  TArray<VCVCable> CableQueue;

  UPROPERTY()
  ALibrary* LibraryActor{nullptr};

  int64_t LastClickedMenuModuleId{-1};
  int currentReturnModuleId{0};
  TMap<int32, ReturnModulePosition> ReturnModulePositions;

  UPROPERTY()
  Uosc3SaveGame* SaveData{nullptr};

  FDPSVGImporter SVGImporter;
  UPROPERTY()
  TMap<FString, UDPSVGAsset*> SVGAssets;
  UPROPERTY()
  TMap<FString, AWidgetSurrogate*> SVGWidgetSurrogates;
  UPROPERTY()
  TMap<FString, UTexture2D*> SVGTextures;
  
  void LoadPatch(FString PatchPath);
  void StartRack(FString PatchPath);
  void RestartRack(FString PatchPath);
  FString PatchPathToBootstrap{""};
public:
  // delegate stuff
  void SubscribeMenuItemSyncedDelegate(AContextMenu* ContextMenu);
  void SubscribeMenuSyncedDelegate(AContextMenu* ContextMenu);
  void SubscribeGrabbableSetDelegate(AGrabbableActor* GrabbableActor);
};
