#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UI/KeyboardWidget.h"

#include "KeyboardKey.generated.h"

class UButton;
class UTextBlock;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKeyReleasedSignature, class UKeyboardKey*, Key);

UCLASS()
class OSC3_API UKeyboardKey : public UUserWidget {
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;	

public:
  FKeyReleasedSignature OnClickedDelegate;

  UPROPERTY(meta = (BindWidget))
  USizeBox* SizeBox;
  UPROPERTY(meta = (BindWidget))
  UButton* Button;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* PrimaryLabel;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* SecondaryLabel;
  
  KeyInfo Data;

  private:
    UFUNCTION()
    void HandleButtonReleased();
};
