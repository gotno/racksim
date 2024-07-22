#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiToggleButton.generated.h"

class UButton;
class UTextBlock;
class UToggleButtonToggle;

UENUM()
enum class FMultiToggleType : int32 {
  Single,
  Multi
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleOneToggledSignature, bool, bIsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleTwoToggledSignature, bool, bIsChecked);

UCLASS()
class RACKSIM_API UMultiToggleButton : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void SynchronizeProperties() override;

protected:
  virtual void NativeConstruct() override;

public:
  void SetPrimaryLabel(const FString& Text);
  void SetToggleLabels(const FString& ToggleOneLabel, const FString& ToggleTwoLabel);
  void EnableMultiToggle();
  void SetToggleOneChecked(bool bIsChecked);
  void SetToggleTwoChecked(bool bIsChecked);
  
  FOnToggleOneToggledSignature OnToggleOneToggledDelegate;
  FOnToggleTwoToggledSignature OnToggleTwoToggledDelegate;

private:
  UFUNCTION()
  void HandleClick();
  UFUNCTION()
  void HandleToggle(UToggleButtonToggle* Toggle, bool bIsChecked);

  FMultiToggleType Type{ FMultiToggleType::Single };

  UPROPERTY(meta = (BindWidget))
  UButton* Button;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* Label;
  FText LabelText;

  UPROPERTY(meta = (BindWidget))
  UToggleButtonToggle* ToggleOne;
  UPROPERTY(meta = (BindWidget))
  UToggleButtonToggle* ToggleTwo;
};
