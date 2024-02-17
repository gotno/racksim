#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RackManager.generated.h"

UCLASS()
class OSC3_API URackManager : public UObject {
	GENERATED_BODY()
    
public:
  void Init();
  void Run();
  void Close();
  
private:
  void Setup();
  void LaunchRack();
  void TerminateRack();
  void Cleanup();
  
  bool bInitd{false};

  FString RackPluginsPath{""};
  FString gtnosftPluginFilename{"gtnosft-2.0.1-win-x64.vcvplugin"};
  FString RackPath{""};
  FString gtnosftPluginPath{""};
  FString RackExecutablePath{""};
  FString OSCctrlBootstrapPath{""};
  FString AutosavePath{""};

  FProcHandle hRackProc;
  FTimerHandle hFocusViewportTimer;
  void BringViewportToFront();
};