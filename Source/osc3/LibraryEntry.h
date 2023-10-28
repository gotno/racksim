#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LibraryEntry.generated.h"

UCLASS()
class OSC3_API ULibraryEntry : public UObject {
	GENERATED_BODY()
	
public:
  FString PluginName;
  FString PluginSlug;
  FString ModuleName;
  FString ModuleSlug;
  FString ModuleDescription;
  TArray<FString> Tags;
};