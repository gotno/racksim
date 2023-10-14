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
	UTextBlock* Label;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock* DisplayValue;

public:
  void SetLabel(FString NewLabel);
  void SetDisplayValue(FString NewDisplayValue);
};