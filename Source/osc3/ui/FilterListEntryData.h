#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FilterListEntryData.generated.h"

enum EFilterType {
  BRAND,
  TAGS
};

UCLASS()
class OSC3_API UFilterListEntryData : public UObject {
	GENERATED_BODY()
	
public:
  FString Label;
  FString StringValue;
  int IntValue;
  bool bSelected{false};
  EFilterType Type;
};