#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "IndicatorHUD.generated.h"

UCLASS()
class OSC3_API AIndicatorHUD : public AHUD {
	GENERATED_BODY()
    
public:
  AIndicatorHUD();
	
protected:
  virtual void DrawHUD() override;

private:
  class UTexture2D* crosshair;
};