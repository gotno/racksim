#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"

#include "osc3.h"

#include "RackSimGameUserSettings.generated.h"

UCLASS()
class RACKSIM_API URackSimGameUserSettings : public UGameUserSettings {
  GENERATED_BODY()

public:
  // cables
  UPROPERTY(Config)
  float CableOpacity;
  UPROPERTY(Config)
  float CableTension;
  UPROPERTY(Config)
  bool bCycleCableColors;
  UPROPERTY(Config)
  int LastCableColorIndex;

  // controller lights
  UPROPERTY(Config)
  bool bShowLeftControllerLight;
  UPROPERTY(Config)
  bool bShowRightControllerLight;

  // controller tooltips
  UPROPERTY(Config)
  bool bShowLeftControllerTooltip;
  UPROPERTY(Config)
  bool bShowRightControllerTooltip;

  UPROPERTY(Config)
  bool bSettingsInitialized;

  bool IsInitialized() {
    return bSettingsInitialized;
  }

  void Init() {
    CableOpacity = DEFAULT_CABLE_OPACITY;
    CableTension = DEFAULT_CABLE_TENSION;
    bCycleCableColors = true;
    LastCableColorIndex = -1;

    bShowLeftControllerLight = true;
    bShowRightControllerLight = true;

    bShowLeftControllerTooltip = true;
    bShowRightControllerTooltip = true;
    
    bSettingsInitialized = true;
  }

  void Print() {
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::CableOpacity: %f"), CableOpacity);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::CableTension: %f"), CableTension);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::bCycleCableColors: %d"), bCycleCableColors);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::LastCableColorIndex: %d"), LastCableColorIndex);
    UE_LOG(LogTemp, Warning, TEXT("\n"));
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::bShowLeftControllerLight: %d"), bShowLeftControllerLight);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::bShowRightControllerLight: %d"), bShowRightControllerLight);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::bShowLeftControllerTooltip: %d"), bShowLeftControllerTooltip);
    UE_LOG(LogTemp, Warning, TEXT("RackGameUserSettings::bShowRightControllerTooltip: %d"), bShowRightControllerTooltip);
  }
};
