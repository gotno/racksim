#include "RackManager.h"

#include "OSCController.h"

#include "Windows/AllowWindowsPlatformTypes.h"
THIRD_PARTY_INCLUDES_START
#include "winuser.h"
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformTypes.h"

#include "Json.h"
#include "Misc/Paths.h"

void URackManager::Init() {
#if !UE_BUILD_SHIPPING
  RackPath = "C:/VCV/UnrealBuild/Windows/osc3_dev/Rack2Free/";
#else
  RackPath = FString(FPlatformProcess::UserHomeDir()) + "Rack2Free/";
#endif
  RackExecutablePath = RackPath + "Rack.exe";
  OSCctrlBootstrapPath = RackPath + "oscctrl-bootstrap.vcv";
  AutosavePath = RackPath + "autosave/patch.json";
  RackUserPath = FString(FPlatformProcess::UserDir()) + "Rack2/";
  RackPluginsPath = RackUserPath + "plugins-win-x64/";
}

void URackManager::Run(bool bNewPatch, TFunction<void ()> inFinishRunCallback) {
  SetupPlugin();
  FinishRunCallback = inFinishRunCallback;
  LaunchRack(bNewPatch);
}

void URackManager::SetupPlugin() {
  IFileManager& FileManager = IFileManager::Get();
  FileManager.Copy(
    *(RackPluginsPath + gtnosftPluginFilename),
    *(RackPath + gtnosftPluginFilename)
  );
}

bool URackManager::DoesAutosaveExist() {
  // TODO?: check for OSCctrl module in autosave
  //        definitely if ever we share the autosave with system rack
  return FPaths::FileExists(AutosavePath);
}

void URackManager::LaunchRack(bool bNewPatch) {
  FString params =
    bNewPatch
      ? OSCctrlBootstrapPath
      : "";

  hRackProc = FPlatformProcess::CreateProc(
    *RackExecutablePath,
    *params,
    true, // launch detached, does nothing?
    false, // launch hidden, does nothing?
    false, // launch RLY hidden, does nothing?
    nullptr,
    2, // -2 to 2 priority, idle to highest
    *RackPath, // working directory
    nullptr,
    nullptr
  );

  if (hRackProc.IsValid()) {
    GetWorld()->GetTimerManager().SetTimer(
      hFinishRunTimer,
      this,
      &URackManager::FinishRun,
      0.1f, // 100 ms
      true // loop
    );
  }
}

void URackManager::FinishRun() {
  // ensure rack is running before continuing
  if (!hRackProc.IsValid()) return;
  if (!FPlatformProcess::IsProcRunning(hRackProc)) return;

  // find rack window before continuing
  LPCSTR rackWindowClass = "GLFW30";
  LPCSTR rackWindowName = "RACK";
  HWND hWnd = FindWindowA(rackWindowClass, rackWindowName);
  if (!hWnd) return;
  
  // ensure we can get the unreal window before continuing
  if (!GEngine || !GEngine->GameViewport) return;
  
  BringViewportToFront();
  bRunning = true;
  FinishRunCallback();

  GetWorld()->GetTimerManager().ClearTimer(hFinishRunTimer);
}

void URackManager::BringViewportToFront() {
  FGenericWindow* window =
    GEngine->GameViewport->GetWindow()->GetNativeWindow().Get();
  window->Minimize();
  window->Restore();
}

void URackManager::Cleanup() {
  // force Rack to close if it is still running
  if (hRackProc.IsValid() && FPlatformProcess::IsProcRunning(hRackProc)) {
    FPlatformProcess::TerminateProc(hRackProc, false);
  }

  FPlatformFileManager::Get()
    .GetPlatformFile()
    .DeleteDirectoryRecursively(*(RackPluginsPath + "/gtnosft"));
}

TArray<FString> URackManager::GetRecentPatchPaths() {
  TArray<FString> recentPaths;

  FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *(RackUserPath + "settings.json")))
    return recentPaths;

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*JsonStr);
  TSharedPtr<FJsonValue> OutJson;

  if (!FJsonSerializer::Deserialize(JsonReader, OutJson))
    return recentPaths;

  TSharedPtr<FJsonObject> rootJ = OutJson->AsObject();

  IFileManager& FileManager = IFileManager::Get();
  for (auto& path : rootJ->GetArrayField(FString("recentPatchPaths")))
    if (FileManager.FileExists(*path->AsString())) recentPaths.Push(path->AsString());

  return recentPaths;
}