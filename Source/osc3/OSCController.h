#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "OSCClient.h"
#include "OSCServer.h"

#include "VCV.h"

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
  void SendParamUpdate(int64 moduleId, int paramId, float value);
  UFUNCTION()
  void CreateCable(int64 inputModuleId, int64 ouputModuleId, int inputPortId, int outputPortId);
  UFUNCTION()
  void DestroyCable(int64 cableId);

private:
  UPROPERTY()
  class Aosc3GameModeBase* gameMode;

  UPROPERTY()
  UOSCClient* OSCClient;

  UPROPERTY()
  UOSCServer* OSCServer;
  
  void AddRoute(const FString &AddressPattern, const FName &MethodName);
  bool ModuleGuard(const FOSCMessage &message, int64_t &moduleId);

  UFUNCTION()
  void LogOSC(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
  UFUNCTION()
  void AddModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port);
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

  // UFUNCTION()
  // void TestBundle(const FOSCBundle& InBundle, const FString& InIPAddress, int32 InPort);
  // UFUNCTION()
  // void TestMessage(const FOSCMessage& InMessage, const FString& InIPAddress, int32 InPort);

  UFUNCTION()
  void NotifyReceived(FString type, int64 outerId, int innerId = -1);
  
  // void PrintVCVModule(VCVModule mod);

  TMap<int64, VCVModule> Modules;
  TMap<int64, VCVCable> Cables;
};