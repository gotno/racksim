#pragma once

#include "CoreMinimal.h"

#include "VCVData/VCV.h"

#include "GameFramework/GameModeBase.h"
#include "osc3GameModeBase.generated.h"

class Aosc3GameState;
class Uosc3GameInstance;
class URackSimGameUserSettings;
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
class Aosc3PlayerController;
class UTexture2D;
class AModuleWeldment;
class USvgRenderer;
class ASkyLight;
class ADirectionalLight;
struct FVCVCableInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTextureReadySignature, FString, Filepath, UTexture2D*, Texture);

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
class RACKSIM_API Aosc3GameModeBase : public AGameModeBase {
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

  void RegisterModule(VCVModule vcv_module);
  void QueueCableSpawn(VCVCable vcv_cable);
  void RequestExit();
  void ToggleMainMenu();
  void SummonLibrary(FVector Location, FRotator Rotation);
  void TuckLibrary();

  AVCVCable* SpawnCable();
  AVCVCable* SpawnCable(AVCVPort* Port);
  void SpawnCable(int64_t& Id, AVCVPort* InputPort, AVCVPort* OutputPort, FLinearColor Color);
  void SpawnCable(FVCVCableInfo& cable_info);
  void DestroyCableActor(AVCVCable* Cable);
  void DestroyCableActor(int64_t& CableId);
  void RegisterCableConnect(AVCVPort* InputPort, AVCVPort* OutputPort, FColor Color);
  void RegisterCableDisconnect(AVCVCable* Cable);

  void UpdateLight(int64_t ModuleId, int32 LightId, FLinearColor Color);
  void UpdateParam(int64_t ModuleId, VCVParam& Param);
  void UpdatePort(int64_t ModuleId, VCVPort& Port);

  void SendParamUpdate(int64_t ModuleId, int32 ParamId, float Value);
  void DuplicateModule(AVCVModule* Module);
  void DestroyModule(int64_t ModuleId, bool bSync = true);
  void RequestModuleSpawn(FString PluginSlug, FString ModuleSlug);
  void RequestModuleDiff(const int64_t& ModuleId) const;
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  void RequestMenu(const FVCVMenu& Menu) const;
  void ClickMenuItem(const FVCVMenuItem& MenuItem);
  void UpdateMenuItemQuantity(const FVCVMenuItem& MenuItem, const float& Value) const;

  void ConfirmSaved();

  TArray<FString> GetRecentPatchPaths();

  void RequestTexture(FString& Filepath, UObject* Requester, const FName& Callback);
  FTextureReadySignature OnTextureReadyDelegate;
  
  void SpawnLibrary();
  void SpawnMainMenu();
  void SetLibraryJsonPath(FString& Path);
  ALibrary* GetLibrary();

  void DestroyWeldment(AModuleWeldment* Weldment);
  void SplitWeldment(AModuleWeldment* Weldment, int AfterIndex);
  void SplitWeldment(AModuleWeldment* Weldment, AVCVModule* OnModule);
  void WeldModules(TArray<int64>& ModuleIds, bool bShouldArrangeRackside = false);
  void WeldModules(AVCVModule* LeftModule, AVCVModule* RightModule, bool bLeftIsAnchor = true, bool bShouldArrangeRackside = true);

  UFUNCTION()
  void SaveUserSettings();

  void AlertGeneratePreviews();
  void AlertGeneratePreviews(TArray<FString> PluginSlugs);

private:
  UFUNCTION()
  void LoadUserSettings();
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

  FTimerHandle hRackRunningTimer;
  void WatchRackStartup();
  UFUNCTION()
  void RackStartupTimeout();
  void WatchRackRunning();
  UFUNCTION()
  void CheckRackRunning();

  UPROPERTY()
  URackManager* rackman;
  UPROPERTY()
  AOSCController* OSCctrl;
  UPROPERTY()
  USvgRenderer* svg2tex;
  UPROPERTY()
  AMainMenu* MainMenu;

  UPROPERTY()
  Aosc3GameState* osc3GameState;
  UPROPERTY()
  Aosc3PlayerController* PlayerController;
  UPROPERTY()
  AVRAvatar* PlayerPawn;
  UPROPERTY()
  Uosc3GameInstance* osc3GameInstance;
  UPROPERTY()
  URackSimGameUserSettings* UserSettings;

  UPROPERTY()
  TMap<int64, AVCVModule*> ModuleActors;

  void RegisterSvg(FString Filepath);
  TArray<FString> SvgsToRender;
  bool bRenderingSvg{false};
  void RenderNextSvg();
  UFUNCTION()
  void AddSvgTexture(FString Filepath, UTexture2D* Texture);

  TArray<VCVModule> ModulesToSpawn;
  bool bSpawningModule{false};
  void SpawnNextModule();
  void SpawnModule(VCVModule vcv_module);

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

  UPROPERTY()
  ASkyLight* FillLight;
  UPROPERTY()
  ADirectionalLight* SunLight;

  int64_t LastClickedMenuModuleId{-1};
  int currentReturnModuleId{0};
  TMap<int32, ReturnModulePosition> ReturnModulePositions;

  UPROPERTY()
  Uosc3SaveGame* SaveData{nullptr};

  UPROPERTY()
  TMap<FString, UTexture2D*> SvgTextures;

  void AdjustLightIntensity(float Amount);
  void AdjustLightAngle(float Amount);

  void AdjustScalingFactor(float Amount);

  void SavePatch(FString PatchPath = "");
  void LoadPatch(FString PatchPath);
  void LoadMap(FString MapName, FString NextPatchPath = "");
  void StartRack(FString PatchPath);
  void RestartRack(FString PatchPath);
  FString PatchPathToBootstrap{""};

  UFUNCTION()
  void HandlePreviewGeneratedStatus(FString Status, bool bSuccess);
public:
  // delegate stuff
  void SubscribeMenuItemSyncedDelegate(AContextMenu* ContextMenu);
  void SubscribeMenuSyncedDelegate(AContextMenu* ContextMenu);
  void SubscribeGrabbableSetDelegate(AGrabbableActor* GrabbableActor);
};
