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
  void SetCanContinueAutosave(bool inbCanContinueAutosave) { bCanContinueAutosave = inbCanContinueAutosave; }
  bool CanContinueAutosave() { return bCanContinueAutosave; }

  void SetPatchPath(FString inPatchPath) {
    PatchPath = inPatchPath;
    SaveName = FMD5::HashAnsiString(*PatchPath);
  }

  FString GetSaveName() {
    return SaveName;
  }

private:
  bool bPatchLoaded;
  bool bIsAutosave;
  bool bCanContinueAutosave;
  FString PatchPath{""};
  FString SaveName{""};
  FString AutosaveName{"autosave"};
};
