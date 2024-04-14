#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "Misc/SecureHash.h"

#include "osc3GameState.generated.h"

UCLASS()
class OSC3_API Aosc3GameState : public AGameStateBase {
	GENERATED_BODY()
	
public:
  bool IsPatchLoaded() { return bPatchLoaded; }
  void SetPatchLoaded(bool inbPatchLoaded) { bPatchLoaded = inbPatchLoaded; }

  bool IsUnsaved() { return bUnsaved; }
  void SetSaved() { bUnsaved = false; }
  void SetUnsaved() { if (IsPatchLoaded()) bUnsaved = true; }

  bool CanContinueAutosave() { return bCanContinueAutosave; }
  void SetCanContinueAutosave(bool inbCanContinueAutosave) { bCanContinueAutosave = inbCanContinueAutosave; }

  void SetPatchPath(FString inPatchPath) {
    PatchPath = inPatchPath;
    if (PatchPath.Equals("new")) {
      SaveName = "";
    } else if (PatchPath.Equals(AutosaveName)) {
      SaveName = AutosaveName;
    } else {
      SaveName = FMD5::HashAnsiString(*PatchPath);
    }
  }

  FString GetSaveName() {
    return SaveName;
  }

  FString GetAutosaveName() {
    return AutosaveName;
  }

private:
  bool bPatchLoaded{false};
  bool bUnsaved{false};
  bool bCanContinueAutosave{false};
  FString PatchPath{""};
  FString SaveName{""};
  FString AutosaveName{"autosave"};
};
