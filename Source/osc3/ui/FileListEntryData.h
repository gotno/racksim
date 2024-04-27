#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "osc3.h"

#include "FileListEntryData.generated.h"

UCLASS()
class OSC3_API UFileListEntryData : public UObject {
	GENERATED_BODY()
	
public:
  FString Label;
  FString Path;
  bool bScrollHover{false};
  EFileType Type{EFileType::File};
  TFunction<void (FString)> ClickCallback;
};