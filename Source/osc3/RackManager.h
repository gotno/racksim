#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RackManager.generated.h"

UCLASS()
class OSC3_API URackManager : public UObject {
	GENERATED_BODY()
    
public:
  void Init();
  void Run(FString PatchPath, TFunction<void ()> inFinishRunCallback);
  void Cleanup();
  bool RackIsRunning() { return bRunning; }
  bool DoesAutosaveExist();
  FString GetBootstrapPath() { return OSCctrlBootstrapPath; }
  TArray<FString> GetRecentPatchPaths();
  void CallOnExit(TFunction<void ()> inOnExitCallback);

  float AutosaveInterval{15.f};
  TArray<FColor> CableColors;

  void GetAutosaveInfo(FString& PatchPath, bool& bIsSaved);
  
private:
  void SetupPlugin();
  void LaunchRack(FString PatchPath);
  void FinishRun();
  TFunction<void ()> FinishRunCallback;
  bool bRunning{false};

  void LoadConfigurationData();

  FString RackUserPath{""};
  FString RackPluginsPath{""};
  FString gtnosftPluginFilename{"gtnosft-2.0.1-win-x64.vcvplugin"};
  FString RackPath{""};
  FString gtnosftPluginPath{""};
  FString RackExecutablePath{""};
  FString OSCctrlBootstrapPath{""};
  FString AutosavePath{""};

  FProcHandle hRackProc;
  FTimerHandle hFinishRunTimer;
  FTimerHandle hOnExitTimer;
  void BringViewportToFront();

  TFunction<void ()> OnExitCallback;
  UFUNCTION()
  void CheckForExit();
};