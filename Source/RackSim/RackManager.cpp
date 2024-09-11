#include "RackManager.h"

#include "OSCController.h"

#include "Windows/AllowWindowsPlatformTypes.h"
THIRD_PARTY_INCLUDES_START
#include "winuser.h"
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformTypes.h"

#include "Json.h"
#include "Internationalization/Regex.h"
#include "Misc/Paths.h"

void URackManager::Init() {
#if !UE_BUILD_SHIPPING
  RackPath = "C:/VCV/UnrealBuild/gsRS2024_dev/Windows/Rack2Free/";
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
  HiddenPluginsDirectory = RackUserPath + "plugins-racksim-hidden/";

  LoadConfigurationData();

  // in case of a messy shutdown while generating previews
  UnhidePlugins();
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

  // add quotes in case the path has spaces
  if (!params.IsEmpty()) params = "\"" + params + "\"";

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
      &URackManager::PipeLogs,
      0.2f, // 200 ms
      true // loop
    );
  }
}

void URackManager::PipeLogs() {
  FString stdOut = FPlatformProcess::ReadPipe(StdOutReadHandle);
  if (stdOut.Len() > 0) UE_LOG(LogTemp, Display, TEXT(" RACKLOG:\n%s"), *stdOut);
}

void URackManager::CallOnExit(TFunction<void ()> inOnExitCallback) {
  OnExitCallback = inOnExitCallback;

  GetWorld()->GetTimerManager().ClearTimer(hOnExitTimer);
  GetWorld()->GetTimerManager().SetTimer(
    hOnExitTimer,
    this,
    &URackManager::CheckForExit,
    0.1f, // 100 ms
    true // loop
  );
}

void URackManager::CancelCallOnExit() {
  GetWorld()->GetTimerManager().ClearTimer(hOnExitTimer);
  OnExitCallback = []() {};
}

void URackManager::CheckForExit() {
  if (RackIsRunning()) return;

  GetWorld()->GetTimerManager().ClearTimer(hPipeReadTimer);
  GetWorld()->GetTimerManager().ClearTimer(hOnExitTimer);
  OnExitCallback();
}

void URackManager::CallOnRunning(TFunction<void ()> inOnRunningCallback) {
  OnRunningCallback = inOnRunningCallback;

  GetWorld()->GetTimerManager().ClearTimer(hOnRunningTimer);
  GetWorld()->GetTimerManager().SetTimer(
    hOnRunningTimer,
    this,
    &URackManager::CheckForRunning,
    0.1f, // 100 ms
    true // loop
  );
}

void URackManager::CheckForRunning() {
  // ensure rack is running
  if (!RackIsRunning()) return;

  GetWorld()->GetTimerManager().ClearTimer(hOnRunningTimer);
  OnRunningCallback();
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

// TODO: CallOnRunning this
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
  // if (!window) return;
  window->Minimize();
  window->Restore();
}

void URackManager::KillRack() {
  if (RackIsRunning()) FPlatformProcess::TerminateProc(hRackProc, false);
}

void URackManager::Cleanup() {
  // force Rack to close if it is still running
  KillRack();

  UnhidePlugins();

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

// need:
// - update warning to indicate rack will close
// - close rack if running
// - menu: add cancel button to status section
void URackManager::RegenerateModulePreviews() {
  FPlatformFileManager::Get()
    .GetPlatformFile()
    .DeleteDirectoryRecursively(*(RackUserPath + "screenshots/"));
  GenerateModulePreviews();
}

void URackManager::GenerateModulePreviews(bool bIsRestart) {
  FPlatformProcess::CreatePipe(StdOutReadHandle, StdOutWriteHandle);

  hRackProc = FPlatformProcess::CreateProc(
    *RackExecutablePath,
    TEXT("-a -t3"), // -a: safemode (no autosave), -t3: screenshot 3x zoom
    false, // launch detached, does nothing?
    false, // launch hidden, does nothing?
    false, // launch RLY hidden, does nothing?
    nullptr,
    2, // -2 to 2 priority, idle to highest
    *RackPath, // working directory
    StdOutWriteHandle, // read pipe
    nullptr // write pipe
  );

  if (hRackProc.IsValid()) {
    LastPreviewModule = "";

    if (!bIsRestart) {
      UE_LOG(LogTemp, Warning, TEXT("Initializing preview generation..."));
      OnPreviewGeneratedStatus.Broadcast(FString("Initializing preview generation..."));
    }

    GetWorld()->GetTimerManager().SetTimer(
      hPipeReadTimer,
      this,
      &URackManager::ReadScreenshottingLog,
      0.2f, // 200 ms
      true // loop
    );

    CallOnRunning([this]() {
      GetWorld()->GetTimerManager().SetTimer(
        hCheckPreviewTimer,
        this,
        &URackManager::CheckPreviewGenerationStalled,
        2.f,
        true
      );
    });
  }
}

void URackManager::CancelPreviewGeneration() {
  ClearPreviewGenerationTimers();

  CallOnExit([this]() {
    UnhidePlugins();
  });
  KillRack();
}

void URackManager::ClearPreviewGenerationTimers() {
  GetWorld()->GetTimerManager().ClearTimer(hCheckPreviewTimer);
  GetWorld()->GetTimerManager().ClearTimer(hPipeReadTimer);
}

void URackManager::ReadScreenshottingLog() {
  FString stdOut = FPlatformProcess::ReadPipe(StdOutReadHandle);
  if (stdOut.Len() == 0) return; 
  
  FRegexPattern crashRegex(TEXT("Fatal signal"));
  FRegexMatcher crashMatcher(crashRegex, stdOut);
  if (crashMatcher.FindNext()) {
    ClearPreviewGenerationTimers();

    FString msg;
    msg.Appendf(TEXT("Previews for plugin %s stalled, skipping..."), *CurrentPreviewPlugin);
    OnPreviewGeneratedStatus.Broadcast(msg);
    UE_LOG(LogTemp, Error, TEXT("Previews for plugin %s stalled, skipping..."), *CurrentPreviewPlugin);

    HidePlugin(CurrentPreviewPlugin);
    GenerateModulePreviews(true);
    return;
  }

  FRegexPattern finishRegex(TEXT("Finished screenshotting modules"));
  FRegexMatcher finishMatcher(finishRegex, stdOut);
  if (finishMatcher.FindNext()) {
    OnPreviewGeneratedStatus.Broadcast(FString("Previews generated!"));
    UE_LOG(LogTemp, Warning, TEXT("Previews generated!"));
    ClearPreviewGenerationTimers();
    UnhidePlugins();
    return;
  }

  FString plugin, module;
  FRegexPattern regex(TEXT("Screenshotting (.*) (.*) to"));
  FRegexMatcher matcher(regex, stdOut);
  while (true) {
    if (!matcher.FindNext()) break;
    plugin = matcher.GetCaptureGroup(1);
    module = matcher.GetCaptureGroup(2);

    CurrentPreviewPlugin = plugin;
    CurrentPreviewModule = module;

    FString msg;
    msg.Appendf(TEXT("Generating preview for %s/%s"), *plugin, *module);
    OnPreviewGeneratedStatus.Broadcast(msg);
    UE_LOG(LogTemp, Warning, TEXT("Generating preview for %s/%s"), *plugin, *module);
  }
}

void URackManager::CheckPreviewGenerationStalled() {
  UE_LOG(LogTemp, Warning, TEXT("Checking preview generation stalled for %s, last checked %s"), *CurrentPreviewModule, *LastPreviewModule);
  if (LastPreviewModule.IsEmpty() || !CurrentPreviewModule.Equals(LastPreviewModule)) {
    LastPreviewModule = CurrentPreviewModule;
    return;
  }
  CancelCallOnExit();
  ClearPreviewGenerationTimers();

  FString msg;
  msg.Appendf(TEXT("Previews for plugin %s stalled, skipping..."), *CurrentPreviewPlugin);
  OnPreviewGeneratedStatus.Broadcast(msg);
  UE_LOG(LogTemp, Error, TEXT("Previews for plugin %s stalled, skipping..."), *CurrentPreviewPlugin);

  CallOnExit([this]() {
    HidePlugin(CurrentPreviewPlugin);
    GenerateModulePreviews(true);
  });
  KillRack();
}

void URackManager::HidePlugin(FString& PluginSlug) {
  IPlatformFile& fm = FPlatformFileManager::Get().GetPlatformFile();
  if (!FPaths::DirectoryExists(HiddenPluginsDirectory))
    fm.CreateDirectory(*(HiddenPluginsDirectory));

  // copy plugin
  fm.CopyDirectoryTree(
    *(HiddenPluginsDirectory + PluginSlug),
    *(RackPluginsPath + PluginSlug),
    true
  );

  // remove original
  if (FPaths::DirectoryExists(HiddenPluginsDirectory + PluginSlug)) {
    fm.DeleteDirectoryRecursively(*(RackPluginsPath + PluginSlug));
  }
}

void URackManager::UnhidePlugins() {
  if (!FPaths::DirectoryExists(HiddenPluginsDirectory)) return;

  TArray<FString> directories;
  IFileManager::Get()
    .FindFiles(directories, *(HiddenPluginsDirectory + "*"), false, true);

  // restore plugins
  for (FString& directory : directories) {
    FString pluginName = FPaths::GetPathLeaf(directory);
    FPlatformFileManager::Get().GetPlatformFile().CopyDirectoryTree(
      *(RackPluginsPath + directory),
      *(HiddenPluginsDirectory + directory),
      true
    );
  }

  // remove copies
  FPlatformFileManager::Get()
    .GetPlatformFile()
    .DeleteDirectoryRecursively(*HiddenPluginsDirectory);
}