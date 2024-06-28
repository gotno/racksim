#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tooltip.generated.h"

class UCommonTextBlock;

UCLASS()
class RACKSIM_API UTooltip : public UUserWidget {
	GENERATED_BODY()

protected:
  UPROPERTY(meta=(BindWidget))
  UCommonTextBlock* LineOne;
  UPROPERTY(meta=(BindWidget))
  UCommonTextBlock* LineTwo;

public:
  void SetText(
    FString LineOne,
    FString LineTwo = FString(""),
    bool bEmphasis = false,
    bool bSubtitle = false
  );
  
private:
  int32 DefaultTextSize{14};
  int32 EmphasisTextSize{20};
  int32 SubtitleTextSize{10};
};