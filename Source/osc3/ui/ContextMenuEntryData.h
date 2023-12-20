#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "VCV.h"

#include "ContextMenuEntryData.generated.h"

UCLASS()
class OSC3_API UContextMenuEntryData : public UObject {
	GENERATED_BODY()
public:
  VCVMenuItem MenuItem;
  bool dividerPrev{false}, dividerNext{false};
};