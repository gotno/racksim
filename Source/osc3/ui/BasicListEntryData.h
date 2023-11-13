#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BasicListEntryData.generated.h"

enum BasicListEntryType {
  BRAND_FILTER,
  TAGS_FILTER
};

UCLASS()
class OSC3_API UBasicListEntryData : public UObject {
	GENERATED_BODY()
	
public:
  FString Label;
  FString StringValue;
  int IntValue;
  bool bSelected{false};
  BasicListEntryType Type;
};