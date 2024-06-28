#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "osc3.h"
#include "Misc/SecureHash.h"

#include "osc3GameState.generated.h"

UCLASS()
class RACKSIM_API Aosc3GameState : public AGameStateBase {
  GENERATED_BODY()

public:
  bool IsPatchLoaded() { return bPatchLoaded; }
  void SetPatchLoaded(bool inbPatchLoaded) { bPatchLoaded = inbPatchLoaded; }
  bool HasSaveFile() {
    return !PatchPath.Equals("new") && !PatchPath.Equals(AutosaveName);
  }

  bool IsUnsaved() { return bUnsaved; }
  void SetSaved() { bUnsaved = false; }
  void SetUnsaved() { if (IsPatchLoaded()) bUnsaved = true; }

  bool CanContinueAutosave() { return bCanContinueAutosave; }
  void SetCanContinueAutosave(bool inbCanContinueAutosave) { bCanContinueAutosave = inbCanContinueAutosave; }

  FString GetPatchPath() {
    if (PatchPath.Equals("new") || PatchPath.Equals(AutosaveName)) {
      return "";
    }
    return PatchPath;
  }

  void SetPatchPath(FString inPatchPath) {
    PatchPath = inPatchPath;
    bPatchIsAutosave = PatchPath.Equals(AutosaveName);

    if (PatchPath.Equals("new")) {
      SaveName = "template";
    } else if (bPatchIsAutosave) {
      SaveName = AutosaveName;
    } else {
      SaveName = PatchPath;
      SaveName.ReplaceInline(TEXT("/"), TEXT("_"), ESearchCase::IgnoreCase);
      SaveName.ReplaceInline(TEXT("\\"), TEXT("_"), ESearchCase::IgnoreCase);
      SaveName.ReplaceInline(TEXT(":"), TEXT("_"), ESearchCase::IgnoreCase);
    }
  }

  bool IsAutosave() { return bPatchIsAutosave; }

  FString GetSaveName() {
    return SaveName;
  }

  FString GetAutosaveName() {
    return AutosaveName;
  }

  float EnvironmentLightIntensityAmount{0.24f};
  float EnvironmentLightAngleAmount{0.45f};
  FString CurrentMapName{"dark_void"};
  void SetCurrentMapName(FString MapName) {
    CurrentMapName = MapName;

    EnvironmentLightIntensityAmount =
      LightIntensityMap.Contains(CurrentMapName)
        ? LightIntensityMap[CurrentMapName]
        : LightIntensityMap[TEXT("default")];

    EnvironmentLightAngleAmount =
      LightAngleMap.Contains(CurrentMapName)
        ? LightAngleMap[CurrentMapName]
        : LightAngleMap[TEXT("default")];
  }

private:
  bool bPatchLoaded{false};
  bool bUnsaved{false};
  bool bCanContinueAutosave{false};
  bool bPatchIsAutosave{false};
  FString PatchPath{""};
  FString SaveName{""};
  FString AutosaveName{"autosave"};

  TMap<FString, float> LightIntensityMap = {
    {"default", DEFAULT_ROOM_BRIGHTNESS_AMOUNT},
    {"park", DEFAULT_ROOM_BRIGHTNESS_AMOUNT_WITH_SUN},
  };
  TMap<FString, float> LightAngleMap = {
    {"default", DEFAULT_SUN_ANGLE_AMOUNT},
  };
};
