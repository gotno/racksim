#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "osc3SaveGame.generated.h"

USTRUCT()
struct FVCVPositionInfo {
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
  FVCVPositionInfo Position;
};

UCLASS()
class OSC3_API Uosc3SaveGame : public USaveGame {
	GENERATED_BODY()
	
public:
  Uosc3SaveGame();

  UPROPERTY(VisibleAnywhere, Category = Base)
  FString SaveSlotName;
  UPROPERTY(VisibleAnywhere, Category = Base)
  uint32 UserIndex;
  
  UPROPERTY(VisibleAnywhere, Category = Base)
  TMap<int64, FVCVModuleInfo> ModuleInfos;
  UPROPERTY(VisibleAnywhere, Category = Base)
  FVCVPositionInfo LibraryPosition;
  UPROPERTY(VisibleAnywhere, Category = Base)
  FVector PlayerLocation{0.f};
};
