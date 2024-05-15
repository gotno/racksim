#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SvgRenderer.generated.h"

class UTexture2D;
class FSvgWorker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTextureRenderedSignature, FString, Filepath, UTexture2D*, Texture);

UCLASS()
class OSC3_API USvgRenderer : public UObject {
	GENERATED_BODY()
	
public:	
  void RenderTextureAsync(const FString& Filepath);
  FTextureRenderedSignature OnTextureRenderedDelegate;

private:
  FTimerHandle hFinished;

  FSvgWorker* Worker;

  UFUNCTION()
  void CheckFinished();
  void MakeTexture();
};
