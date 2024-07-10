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
  RackPath = "C:/VCV/UnrealBuild/racksim_dev/Windows/Rack2Free/";
#else
  RackPath = FString(FPlatformProcess::UserHomeDir()) + "Rack2Free/";
#endif
  RackExecutablePath = RackPath + "Rack.exe";
  OSCctrlBootstrapPath = RackPath + "oscctrl-bootstrap.vcv";

  RackUserPath = FString(FPlatformProcess::UserSettingsDir()) + "Rack2/";
  if (!FPaths::DirectoryExists(RackUserPath)) // fallback for rack < 2.5
    RackUserPath = FString(FPlatformProcess::UserDir()) + "Rack2/";

  AutosavePath = RackUserPath + "autosave-racksim/patch.json";

  TemplatePath = RackUserPath + "template-racksim.vcv";
  if (!FPaths::FileExists(TemplatePath)) // copy empty template
    IFileManager::Get().Copy(*TemplatePath, *(RackPath + "template.vcv"));

  RackPluginsPath = RackUserPath + "plugins-win-x64/";

  LoadConfigurationData();
}

void URackManager::Run(FString PatchPath, TFunction<void ()> inFinishRunCallback) {
  SetupPlugin();
  FinishRunCallback = inFinishRunCallback;
  LaunchRack(PatchPath);
}

void URackManager::LaunchRack(FString PatchPath) {
  FString params;
  if (PatchPath.Equals("new")) {
    params = OSCctrlBootstrapPath;
  } else if (PatchPath.Equals("autosave")) {
    params = "";
  } else {
    params = PatchPath;
  }

  params.Append(" -d");

  FPlatformProcess::CreatePipe(StdOutReadHandle, StdOutWriteHandle);

  hRackProc = FPlatformProcess::CreateProc(
    *RackExecutablePath,
    *params,
    true, // launch detached, does nothing?
    false, // launch hidden, does nothing?
    false, // launch RLY hidden, does nothing?
    nullptr,
    2, // -2 to 2 priority, idle to highest
    *RackPath, // working directory
    StdOutWriteHandle, // read pipe
    nullptr // write pipe
  );

  if (hRackProc.IsValid()) {
    GetWorld()->GetTimerManager().SetTimer(
      hFinishRunTimer,
      this,
      &URackManager::FinishRun,
      0.1f, // 100 ms
      true // loop
    );

    GetWorld()->GetTimerManager().SetTimer(
      hPipeReadTimer,
      this,
      &URackManager::ReadStdOut,
      0.5f, // 500 ms
      true // loop
    );
  }
}

void URackManager::ReadStdOut() {
  FString StdOut = FPlatformProcess::ReadPipe(StdOutReadHandle);
  if (StdOut.Len() > 0) UE_LOG(LogTemp, Display, TEXT("%s"), *StdOut);
}

void URackManager::CallOnExit(TFunction<void ()> inOnExitCallback) {
  OnExitCallback = inOnExitCallback;

  GetWorld()->GetTimerManager().ClearTimer(hPipeReadTimer);
  GetWorld()->GetTimerManager().SetTimer(
    hOnExitTimer,
    this,
    &URackManager::CheckForExit,
    0.1f, // 100 ms
    true // loop
  );
}

void URackManager::CheckForExit() {
  if (RackIsRunning()) return;

  GetWorld()->GetTimerManager().ClearTimer(hOnExitTimer);
  OnExitCallback();
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

void URackManager::FinishRun() {
  // ensure rack is running before continuing
  if (!RackIsRunning()) return;

  // find rack window before continuing
  LPCSTR rackWindowClass = "GLFW30";
  LPCSTR rackWindowName = "RACK";
  HWND hWnd = FindWindowA(rackWindowClass, rackWindowName);
  if (!hWnd) return;
  
  // ensure we can get the unreal window before continuing
  if (!GEngine || !GEngine->GameViewport) return;
  
  BringViewportToFront();
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
  if (RackIsRunning()) FPlatformProcess::TerminateProc(hRackProc, false);

  FPlatformFileManager::Get()
    .GetPlatformFile()
    .DeleteDirectoryRecursively(*(RackPluginsPath + "/gtnosft"));
}

void URackManager::GetAutosaveInfo(FString& PatchPath, bool& bIsSaved) {
  FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *AutosavePath))
    return;

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*JsonStr);
  TSharedPtr<FJsonValue> OutJson;

  if (!FJsonSerializer::Deserialize(JsonReader, OutJson))
    return;

  TSharedPtr<FJsonObject> rootJ = OutJson->AsObject();

  FString path;
  rootJ->TryGetStringField(TEXT("path"), path);
  PatchPath = path.Equals(TemplatePath) ? "" : path;

  // TODO: this doesn't work because we're not properly handling the history rackside
  bool bUnsaved;
  if (rootJ->TryGetBoolField(TEXT("unsaved"), bUnsaved))
    bIsSaved = !bUnsaved;
}

void URackManager::LoadConfigurationData() {
  FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *(RackUserPath + "settings.json")))
    return;

  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*JsonStr);
  TSharedPtr<FJsonValue> OutJson;

  if (!FJsonSerializer::Deserialize(JsonReader, OutJson))
    return;

  TSharedPtr<FJsonObject> rootJ = OutJson->AsObject();

  for (auto& colorHex : rootJ->GetArrayField(TEXT("cableColors")))
    CableColors.Push(FColor::FromHex(colorHex->AsString()));

  AutosaveInterval = rootJ->GetNumberField(TEXT("autosaveInterval"));
}

TArray<FString> URackManager::GetRecentPatchPaths() {
  TArray<FString> recentPaths;
  recentPaths.Reserve(10);

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

  while (recentPaths.Num() < 10) recentPaths.Emplace("");

  return recentPaths;
}