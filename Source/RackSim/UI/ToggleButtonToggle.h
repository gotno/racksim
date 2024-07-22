#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ToggleButtonToggle.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnToggledSignature, UToggleButtonToggle*, Toggle, bool, bIsChecked);

UCLASS()
class RACKSIM_API UToggleButtonToggle : public UUserWidget {
  GENERATED_BODY()
    
  friend class UMultiToggleButton;

public:
  virtual void SynchronizeProperties() override;

protected:
  virtual void NativeConstruct() override;

private:
  UFUNCTION()
  void HandleClick();

  FOnToggledSignature OnToggledDelegate;

  UPROPERTY(meta = (BindWidget))
  UButton* Button;

  UPROPERTY(meta = (BindWidget))
  UTextBlock* Label;
  void SetLabel(const FString& Text);
  FText LabelText;

  UPROPERTY(meta = (BindWidget))
  UImage* IconChecked;
  UPROPERTY(meta = (BindWidget))
  UImage* IconUnchecked;

  bool bChecked{true};
  bool IsChecked() { return bChecked; }
  void SetChecked(bool inbChecked);
  void ToggleChecked();
};
