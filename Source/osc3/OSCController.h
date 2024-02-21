#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "OSCClient.h"
#include "OSCServer.h"

#include "VCVData/VCV.h"

#include "OSCController.generated.h"

class Aosc3GameModeBase;

UCLASS()
class OSC3_API AOSCController : public AActor {
	GENERATED_BODY()
	
public:	
	AOSCController();

public:	
  UFUNCTION()
  void Init();

  UFUNCTION()
  void NotifyResync();

  UFUNCTION()
  void SendParamUpdate(int64 ModuleId, int ParamId, float Value);
  UFUNCTION()
  void SendCreateModule(const FString& PluginSlug, const FString& ModuleSlug, const int& ReturnId);
  UFUNCTION()
  void SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite);
  UFUNCTION()
  void SendDestroyModule(int64 ModuleId);
  UFUNCTION()
  void CreateCable(int64 InputModuleId, int64 OuputModuleId, int InputPortId, int OutputPortId);
  UFUNCTION()
  void DestroyCable(int64 CableId);
  UFUNCTION()
  void SendAutosaveAndExit();

  void RequestMenu(const VCVMenu& Menu) const;
  void ClickMenuItem(const VCVMenuItem& MenuItem) const;
  void UpdateMenuItemQuantity(const VCVMenuItem& MenuItem, const float& Value) const;
  void SendModuleDiffRequest(const int64_t& ModuleId) const;

private:
  FTimerHandle hSyncPortTimer;
  int MinRackClientPort{7000}, RackClientPort{7000}, MaxRackClientPort{7020};
  int ServerPort{7001};

  UPROPERTY()
  class Aosc3GameModeBase* GameMode;

  UPROPERTY()
  UOSCClient* OSCClient;

  UPROPERTY()
  UOSCServer* OSCServer;
  
  void AddRoute(const FString &AddressPattern, const FName &MethodName);
  bool ModuleGuard(const FOSCMessage &Message, int64_t &ModuleId);

  UFUNCTION()
  void SyncPorts();

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

  void AddPort(const FOSCMessage &Message, VCVPort& vcv_port);

  UFUNCTION()
  void SetLibraryJsonPath(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  UFUNCTION()
  void AddContextMenuItem(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void MenuSynced(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  UFUNCTION()
  void SetClientPort(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);

  // UFUNCTION()
  // void TestBundle(const FOSCBundle& InBundle, const FString& InIPAddress, int32 InPort);
  // UFUNCTION()
  // void TestMessage(const FOSCMessage& InMessage, const FString& InIPAddress, int32 InPort);

  UFUNCTION()
  void NotifyReceived(FString Type, int64 OuterId, int InnerId = -1);
  
  // void PrintVCVModule(VCVModule mod);

  TMap<int64, VCVModule> Modules;
};