#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "WidgetSurrogate.generated.h"

class Aosc3GameModeBase;
class UWidgetComponent;
class UTexture2D;
class UDPSVGAsset;

UCLASS()
class OSC3_API AWidgetSurrogate : public AActor {
	GENERATED_BODY()
	
public:	
	AWidgetSurrogate();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  void SetSVG(UDPSVGAsset* SvgAsset, Vec2 Size, FString Filepath);

private:
  Aosc3GameModeBase* GameMode;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  USceneComponent* SceneComponent;

  UPROPERTY(VisibleAnywhere)
  UWidgetComponent* WidgetComponent;
  UPROPERTY()
  UMaterialInterface* WidgetMaterialInterface;

  UPROPERTY()
  UTexture2D* Texture;

  FString SvgFilepath;
  float DrawSizeScale{150.f};
};