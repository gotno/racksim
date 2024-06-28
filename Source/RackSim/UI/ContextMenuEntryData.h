#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "VCVData/VCV.h"

#include "ContextMenuEntryData.generated.h"

class AContextMenu;

UCLASS()
class RACKSIM_API UContextMenuEntryData : public UObject {
	GENERATED_BODY()
public:
  AContextMenu* ContextMenu;
  FVCVMenuItem MenuItem;
  bool DividerNext{false};
  
  // for type: BACK
  int ParentMenuId;
};