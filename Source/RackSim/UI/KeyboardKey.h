#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UI/KeyboardWidget.h"

#include "KeyboardKey.generated.h"

class UButton;
class UTextBlock;
class USizeBox;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKeyReleasedSignature, class UKeyboardKey*, Key);

UCLASS()
class RACKSIM_API UKeyboardKey : public UUserWidget {
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

  UPROPERTY(meta = (BindWidget))
  UImage* BackspaceIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* ConfirmIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* ShiftIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* ShiftLockIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* SpaceIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* HomeIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* LeftIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* RightIcon;
  UPROPERTY(meta = (BindWidget))
  UImage* EndIcon;
  
  KeyInfo Data;

  private:
    UFUNCTION()
    void HandleButtonReleased();
};
