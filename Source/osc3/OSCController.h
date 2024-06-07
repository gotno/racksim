#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "OSCClient.h"
#include "OSCServer.h"

#include "VCVData/VCV.h"

#include "OSCController.generated.h"

class Aosc3GameModeBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMenuItemSyncedSignature, FVCVMenuItem /* MenuItem */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMenuSyncedSignature, FVCVMenu /* Menu */);

UCLASS()
class OSC3_API AOSCController : public AActor {
	GENERATED_BODY()
	
public:	
	AOSCController();

public:	
  UFUNCTION()
  void Init();

  UFUNCTION()
  void SyncPorts();

  bool IsRunning() { return bRunning; }
  bool IsConnected() { return bConnected; }
  void PauseSending() { bSendingPaused = true; }
  void UnpauseSending() { bSendingPaused = false; }

  UFUNCTION()
  void NotifyResync();

  UFUNCTION()
  void SendParamUpdate(int64 ModuleId, int ParamId, float Value);
  UFUNCTION()
  void SendCreateModule(const FString& PluginSlug, const FString& ModuleSlug, const int& ReturnId);
  UFUNCTION()
  void SendSetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  UFUNCTION()
  void SendDestroyModule(int64 ModuleId);
  UFUNCTION()
  void SendCreateCable(int64 InputModuleId, int64 OuputModuleId, int InputPortId, int OutputPortId, FColor Color);
  UFUNCTION()
  void SendDestroyCable(int64 CableId);
  UFUNCTION()
  void SendAutosaveAndExit(FString NextPatchPath = TEXT(""));
  UFUNCTION()
  void SendSavePatch(FString PatchPath = "");
  UFUNCTION()
  void SendLoadPatch(FString PatchPath);
  UFUNCTION()
  void SendArrangeModules(int64 LeftModuleId, int64 RightModuleId, bool bAttach);
  UFUNCTION()
  void NotifyReceived(FString Type, int64 OuterId, int InnerId = -1);

  void SendMenuRequest(const FVCVMenu& Menu) const;
  void SendMenuItemClick(const FVCVMenuItem& MenuItem) const;
  void SendMenuItemQuantityUpdate(const FVCVMenuItem& MenuItem, const float& Value) const;
  void SendModuleDiffRequest(const int64_t& ModuleId) const;
  void ClearData() {
    Modules.Empty();
  }
private:
  FTimerHandle hSyncPortTimer;
  int MinRackClientPort{7000}, RackClientPort{7000}, MaxRackClientPort{7020};
  int ServerPort{7001};
  bool bRunning{false};
  bool bConnected{false};
  bool bSendingPaused{false};

  UPROPERTY()
  class Aosc3GameModeBase* GameMode;

  UPROPERTY()
  UOSCClient* OSCClient;

  UPROPERTY()
  UOSCServer* OSCServer;
  
  void AddRoute(const FString &AddressPattern, const FName &MethodName);
  bool ModuleGuard(const FOSCMessage &Message, int64_t &ModuleId);

  UFUNCTION()
  void SendServerPort();

  UFUNCTION()
  void LogOSC(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void DestroyModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddParam(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddInput(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddOutput(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddDisplay(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddCable(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddLight(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void ModuleSyncComplete(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void UpdateLight(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void SyncParam(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void SyncPort(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 _port);

  void AddPort(const FOSCMessage &Message, VCVPort& vcv_port);

  UFUNCTION()
  void SetLibraryJsonPath(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  UFUNCTION()
  void AddContextMenuItem(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void MenuSynced(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  UFUNCTION()
  void SetClientPort(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  UFUNCTION()
  void ConfirmSaved(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  // UFUNCTION()
  // void TestBundle(const FOSCBundle& InBundle, const FString& InIPAddress, int32 InPort);
  // UFUNCTION()
  // void TestMessage(const FOSCMessage& InMessage, const FString& InIPAddress, int32 InPort);
  
  // void PrintVCVModule(VCVModule mod);

  TMap<int64, VCVModule> Modules;

public:
  // delegates
  FOnMenuItemSyncedSignature OnMenuItemSyncedDelegate;
  FOnMenuSyncedSignature OnMenuSyncedDelegate;
};