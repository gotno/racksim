#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Misc/InteractiveProcess.h"

#include "RackManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPreviewGeneratedSignature, FString /* StatusText */, bool /* bSuccess */);

UCLASS()
class RACKSIM_API URackManager : public UObject {
  GENERATED_BODY()

public:
  void Init();
  void Run(FString PatchPath, TFunction<void ()> inFinishRunCallback);
  // generate previews for all plugins
  void GenerateModulePreviews();
  // delete previews for these plugins, then generate
  void GenerateModulePreviews(TArray<FString> PluginSlugs);
  // delete previews for all plugins, then generate
  void RegenerateModulePreviews();
  void CancelPreviewGeneration();
  void Cleanup();
  bool DoesAutosaveExist();
  FString GetBootstrapPath() { return OSCctrlBootstrapPath; }
  TArray<FString> GetRecentPatchPaths();
  void CallOnExit(TFunction<void ()> inOnExitCallback);
  void CallOnRunning(TFunction<void ()> inOnRunningCallback);

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
  void KillRack();
  void FinishRun();
  TFunction<void ()> FinishRunCallback;

  void LoadConfigurationData();

  FString RackUserPath{""};
  FString RackPluginsPath{""};
  FString gtnosftPluginFilename{"gtnosft-2.0.1-win-x64.vcvplugin"};
  FString RackPath{""};
  FString HiddenPluginsDirectory{""};
  FString gtnosftPluginPath{""};
  FString RackExecutablePath{""};
  FString OSCctrlBootstrapPath{""};
  FString AutosavePath{""};
  FString TemplatePath{""};

  FProcHandle hRackProc;
  FTimerHandle hFinishRunTimer;
  FTimerHandle hOnExitTimer;
  FTimerHandle hOnRunningTimer;
  void BringViewportToFront();

  void* StdOutReadHandle{nullptr};
  void* StdOutWriteHandle{nullptr};
  FTimerHandle hPipeReadTimer;
  void PipeLogs();
  void ReadScreenshottingLog();

  TFunction<void ()> OnExitCallback;
  UFUNCTION()
  void CheckForExit();
  void CancelCallOnExit();

  TFunction<void ()> OnRunningCallback;
  UFUNCTION()
  void CheckForRunning();

  FString CurrentPreviewPlugin, CurrentPreviewModule{""}, LastPreviewModule{""};
  FTimerHandle hCheckPreviewTimer;
  void ClearPreviewGenerationTimers();
  void CheckPreviewGenerationStalled();
  void HidePlugin(FString& PluginSlug);
  void UnhidePlugins();

// delegate stuff
public:
  FOnPreviewGeneratedSignature OnPreviewGeneratedStatus;
};