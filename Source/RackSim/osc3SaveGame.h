#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "VCVData/VCV.h"

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
struct FVCVCableInfo {
  GENERATED_BODY()

  bool IsConnected() {
    return ModuleId != -1;
  }

  void Print() {
    if (IsConnected()) {
      UE_LOG(LogTemp, Warning, TEXT("connected incomplete cable:"));
      UE_LOG(LogTemp, Warning, TEXT("  moduleId: %lld, portId: %d, type: %d"), ModuleId, PortId, (int)Type);
      UE_LOG(LogTemp, Warning, TEXT("  other end loc: %s, rot: %s"), *OtherEndPosition.Location.ToString(), *OtherEndPosition.Rotation.ToString());
    } else {
      UE_LOG(LogTemp, Warning, TEXT("unconnected incomplete cable:"));
      UE_LOG(LogTemp, Warning, TEXT("  endA loc: %s, rot: %s"), *EndAPosition.Location.ToString(), *EndAPosition.Rotation.ToString());
      UE_LOG(LogTemp, Warning, TEXT("  endB loc: %s, rot: %s"), *EndBPosition.Location.ToString(), *EndBPosition.Rotation.ToString());
    }
  }

  // used when one end is connected
  UPROPERTY()
  int64 ModuleId{-1};
  UPROPERTY()
  int PortId{-1};
  UPROPERTY()
  PortType Type{PortType::Any};
  UPROPERTY()
  FPositionInfo OtherEndPosition;

  // used when neither end is connected
  UPROPERTY()
  FPositionInfo EndAPosition;
  UPROPERTY()
  FPositionInfo EndBPosition;

  UPROPERTY()
  bool bRestored{false};

  UPROPERTY()
  FColor Color;
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
class RACKSIM_API Uosc3SaveGame : public USaveGame {
	GENERATED_BODY()
	
public:
  UPROPERTY(VisibleAnywhere, Category = Base)
  TMap<int64, FVCVModuleInfo> ModuleInfos;
  UPROPERTY(VisibleAnywhere, Category = Base)
  TArray<FVCVCableInfo> IncompleteCables;
  UPROPERTY(VisibleAnywhere, Category = Base)
  TArray<FWeldmentInfo> WeldmentInfos;
  UPROPERTY(VisibleAnywhere, Category = Base)
  FPositionInfo LibraryPosition;
  UPROPERTY(VisibleAnywhere, Category = Base)
  bool bLibraryHidden{true};
  UPROPERTY(VisibleAnywhere, Category = Base)
  FVector PlayerLocation{0.f};
  UPROPERTY(VisibleAnywhere, Category = Base)
  FRotator PlayerRotation{0.f};
  UPROPERTY(VisibleAnywhere, Category = Base)
  bool bAutosavePatchIsSaved{false};
  UPROPERTY(VisibleAnywhere, Category = Base)
  FString PatchPath{""};

  UPROPERTY(VisibleAnywhere, Category = Base)
  FString MapName{""};
  UPROPERTY(VisibleAnywhere, Category = Base)
  float EnvironmentLightIntensityAmount{-1.f};
  UPROPERTY(VisibleAnywhere, Category = Base)
  float EnvironmentLightAngleAmount{-1.f};

  UPROPERTY(VisibleAnywhere, Category = Base)
  float ScalingFactorAmount{-1.f};
};
