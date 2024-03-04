#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "osc3GameState.generated.h"

UCLASS()
class OSC3_API Aosc3GameState : public AGameStateBase {
	GENERATED_BODY()
	
public:
  bool IsPatchLoaded() { return bPatchLoaded; }
  void SetPatchLoaded(bool inbPatchLoaded) { bPatchLoaded = inbPatchLoaded; }
  void SetIsAutosave(bool inbIsAutosave) { bIsAutosave = inbIsAutosave; }
  void SetSaveName(FString inSaveName) { SaveName = inSaveName; }
  void SetCanContinueAutosave(bool inbCanContinueAutosave) { bCanContinueAutosave = inbCanContinueAutosave; }
  bool CanContinueAutosave() { return bCanContinueAutosave; }

private:
  bool bPatchLoaded;
  bool bIsAutosave;
  bool bCanContinueAutosave;
  FString PatchPath;
  FString SaveName;
};
