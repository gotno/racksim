#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SvgRenderer.generated.h"

class UTexture2D;

UCLASS()
class OSC3_API USvgRenderer : public UObject {
	GENERATED_BODY()
	
public:	
  UTexture2D* GetTexture(const FString& Filepath);
};
