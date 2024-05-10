#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "osc3SaveGame.generated.h"

USTRUCT()
struct FPositionInfo {
  GENERATED_BODY()

  UPROPERTY()
  FVector Location{0.f};
  UPROPERTY()
  FRotator Rotation{0.f};
};

USTRUCT()
struct FVCVModuleInfo {
  GENERATED_BODY()
    
  UPROPERTY()
  FPositionInfo Position;
};

USTRUCT()
struct FWeldmentInfo {
  GENERATED_BODY()

  UPROPERTY()
  bool bRestored{false};

  UPROPERTY()
  TArray<int64> ModuleIds;

  UPROPERTY()
  FPositionInfo Position;
};

UCLASS()
class OSC3_API Uosc3SaveGame : public USaveGame {
	GENERATED_BODY()
	
public:
  UPROPERTY(VisibleAnywhere, Category = Base)
  TMap<int64, FVCVModuleInfo> ModuleInfos;
  UPROPERTY(VisibleAnywhere, Category = Base)
  TArray<FWeldmentInfo> WeldmentInfos;
  UPROPERTY(VisibleAnywhere, Category = Base)
  FPositionInfo LibraryPosition;
  UPROPERTY(VisibleAnywhere, Category = Base)
  bool bLibraryHidden{true};
  UPROPERTY(VisibleAnywhere, Category = Base)
  FVector PlayerLocation{0.f};
  UPROPERTY(VisibleAnywhere, Category = Base)
  bool bAutosavePatchIsSaved{false};
};
