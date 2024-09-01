#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Misc/InteractiveProcess.h"

#include "RackManager.generated.h"

UCLASS()
class RACKSIM_API URackManager : public UObject {
  GENERATED_BODY()

public:
  void Init();
  void Run(FString PatchPath, TFunction<void ()> inFinishRunCallback);
  void Cleanup();
  bool DoesAutosaveExist();
  FString GetBootstrapPath() { return OSCctrlBootstrapPath; }
  TArray<FString> GetRecentPatchPaths();
  void CallOnExit(TFunction<void ()> inOnExitCallback);

  FProcHandle GetHandle() { return hRackProc; }
  void SetHandle(FProcHandle inHandle) {
    if (inHandle.IsValid() && FPlatformProcess::IsProcRunning(inHandle)) {
      hRackProc = inHandle;
    }
  }
  bool RackIsRunning() {
    return hRackProc.IsValid() && FPlatformProcess::IsProcRunning(hRackProc);
  }

  FString GetUserPath() {
    return RackUserPath;
  }

  float AutosaveInterval{15.f};
  TArray<FColor> CableColors;

  void GetAutosaveInfo(FString& PatchPath, bool& bIsSaved);
  FString GetTemplatePath() {
    return TemplatePath;
  }

private:
  void SetupPlugin();
  void LaunchRack(FString PatchPath);
  void FinishRun();
  TFunction<void ()> FinishRunCallback;

  void LoadConfigurationData();

  FString RackUserPath{""};
  FString RackPluginsPath{""};
  FString gtnosftPluginFilename{"gtnosft-2.0.1-win-x64.vcvplugin"};
  FString RackPath{""};
  FString gtnosftPluginPath{""};
  FString RackExecutablePath{""};
  FString OSCctrlBootstrapPath{""};
  FString AutosavePath{""};
  FString TemplatePath{""};

  FProcHandle hRackProc;
  FTimerHandle hFinishRunTimer;
  FTimerHandle hOnExitTimer;
  void BringViewportToFront();

  void* StdOutReadHandle{nullptr};
  void* StdOutWriteHandle{nullptr};
  FTimerHandle hPipeReadTimer;
  void ReadStdOut();

  TFunction<void ()> OnExitCallback;
  UFUNCTION()
  void CheckForExit();
};