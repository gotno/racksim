#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FileListEntryData.generated.h"

UCLASS()
class OSC3_API UFileListEntryData : public UObject {
	GENERATED_BODY()
	
public:
  FString Label;
  FString Path;
  TFunction<void (FString)> ClickCallback;
};