#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Tooltip.generated.h"

class UTextBlock;

UCLASS()
class OSC3_API UTooltip : public UUserWidget {
	GENERATED_BODY()

protected:
	// virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* LineOne;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* LineTwo;

public:
  void SetLineOne(FString NewLineOne);
  void SetLineTwo(FString NewLineTwo);
  void SetText(FString LineOne, FString LineTwo = FString(""), bool bEmphasis = false, bool bSubtitle = false);
  
private:
  int32 DefaultTextSize{14};
  int32 EmphasisTextSize{20};
  int32 SubtitleTextSize{10};
};