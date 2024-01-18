#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "VCVData/VCV.h"

#include "ContextMenuEntryData.generated.h"

class AVCVModule;

UCLASS()
class OSC3_API UContextMenuEntryData : public UObject {
	GENERATED_BODY()
public:
  AVCVModule* Module;
  VCVMenuItem MenuItem;
  bool DividerNext{false};
  
  // for type: BACK
  int ParentMenuId;
};