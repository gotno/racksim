#include "RackManager.h"

#include "OSCController.h"

#include "Windows/AllowWindowsPlatformTypes.h"
THIRD_PARTY_INCLUDES_START
#include "winuser.h"
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformTypes.h"

void URackManager::Init() {
  UE_LOG(LogTemp, Warning, TEXT("RACKMAN INNIT?"));

#if !UE_BUILD_SHIPPING
  RackPath = "C:/VCV/UnrealBuild/Windows/osc3_dev/Rack2Free/";
#else
  RackPath = FString(FPlatformProcess::UserHomeDir()) + "Rack2Free/";
#endif
  RackExecutablePath = RackPath + "Rack.exe";
  OSCctrlBootstrapPath = RackPath + "oscctrl-bootstrap.vcv";
  AutosavePath = RackPath + "autosave";
  RackPluginsPath = FString(FPlatformProcess::UserDir()) + "Rack2/plugins-win-x64/";
  
  bInitd = true;
}

void URackManager::Run() {
  if (!bInitd) Init();
  Setup();
  LaunchRack();
}

void URackManager::Setup() {
  IFileManager& FileManager = IFileManager::Get();
  FileManager.Copy(
    *(RackPluginsPath + gtnosftPluginFilename),
    *(RackPath + gtnosftPluginFilename)
  );
}

void URackManager::LaunchRack() {
  FString params =
    IFileManager::Get().DirectoryExists(*AutosavePath)
      ? ""
      : OSCctrlBootstrapPath;

  hRackProc = FPlatformProcess::CreateProc(
    *RackExecutablePath,
    *params,
    // *oscctrlBootstrapPath,
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
      hFocusViewportTimer,
      this,
      &URackManager::BringViewportToFront,
      0.1f, // 100 ms
      true // loop
    );
  }
}

void URackManager::BringViewportToFront() {
  if (!hRackProc.IsValid()) return;
  if (!FPlatformProcess::IsProcRunning(hRackProc)) return;
  if (!GEngine || !GEngine->GameViewport) return;

  LPCSTR rackWindowClass = "GLFW30";
  LPCSTR rackWindowName = "RACK";
  HWND hWnd = FindWindowA(rackWindowClass, rackWindowName);
  if (hWnd) {
    FGenericWindow* window = GEngine->GameViewport->GetWindow()->GetNativeWindow().Get();
    window->Minimize();
    window->Restore();
    // window->Maximize();
    // window->BringToFront(true);
    // window->SetWindowFocus();

    GetWorld()->GetTimerManager().ClearTimer(hFocusViewportTimer);
  }
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