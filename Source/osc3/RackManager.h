#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RackManager.generated.h"

UCLASS()
class OSC3_API URackManager : public UObject {
	GENERATED_BODY()
    
public:
  void Init();
  void Run(bool bNewPatch, TFunction<void ()> inFinishRunCallback);
  void Cleanup();
  bool RackIsRunning() { return bRunning; }
  bool DoesAutosaveExist();
  
private:
  void SetupPlugin();
  void LaunchRack(bool bNewPatch);
  void FinishRun();
  TFunction<void ()> FinishRunCallback;
  
  bool bRunning{false};

  FString RackPluginsPath{""};
  FString gtnosftPluginFilename{"gtnosft-2.0.1-win-x64.vcvplugin"};
  FString RackPath{""};
  FString gtnosftPluginPath{""};
  FString RackExecutablePath{""};
  FString OSCctrlBootstrapPath{""};
  FString AutosavePath{""};

  FProcHandle hRackProc;
  FTimerHandle hFinishRunTimer;
  void BringViewportToFront();
};