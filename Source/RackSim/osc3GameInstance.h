#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "osc3SaveGame.h"

#include "osc3GameInstance.generated.h"

UCLASS()
class RACKSIM_API Uosc3GameInstance : public UGameInstance {
  GENERATED_BODY()

public:
  UPROPERTY()
  Uosc3SaveGame* SaveData{nullptr};

  FProcHandle hRackProc;
  bool RackIsRunning() {
    return hRackProc.IsValid() && FPlatformProcess::IsProcRunning(hRackProc);
  }

  // map switching
  FString NextMapName{""};
  FString NextPatchPath{""};

  UPROPERTY()
  FName LightVoidMap = FName("light_void");
  UPROPERTY()
  FName DarkVoidMap = FName("dark_void");
  UPROPERTY()
  FName ParkMap = FName("park");

  UPROPERTY()
  TMap<FString, FName> MapMap = {
    {"light_void", LightVoidMap},
    {"dark_void", DarkVoidMap},
    {"park", ParkMap}
  };

  FName GetMapName() {
    return MapMap[NextMapName];
  }
};
