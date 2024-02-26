#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "VCVData/VCV.h"

#include "ContextMenuEntry.generated.h"

class Aosc3GameModeBase;
class AVCVModule;
class UButton;
class UTextBlock;
class UCommonTextBlock;
class UBorder;
class USlider;

UCLASS()
class OSC3_API UContextMenuEntry : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;	
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
	UBorder* ActionContainer;
  UPROPERTY(meta = (BindWidget))
	UButton* ActionButton;
  UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* ActionButtonText;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionButtonTextStill;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedIndicator;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SubmenuIndicator;

  UPROPERTY(meta = (BindWidget))
	UBorder* LabelContainer;
  UPROPERTY(meta = (BindWidget))
	UButton* Label;
  UPROPERTY(meta = (BindWidget))
	UCommonTextBlock* LabelText;

  UPROPERTY(meta = (BindWidget))
	UBorder* Range;
  UPROPERTY(meta = (BindWidget))
	USlider* Slider;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SliderText;

  UPROPERTY(meta = (BindWidget))
	UBorder* BackContainer;
  UPROPERTY(meta = (BindWidget))
	UButton* BackButton;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* BackButtonText;
private:
  Aosc3GameModeBase* GameMode;
  VCVMenuItem MenuItem;
  AVCVModule* Module;
  
  // for type: BACK
  int ParentMenuId;
  
  float ActionIndicatorsMargin{24.f};
  
  UFUNCTION()
  void HandleClick();
  UFUNCTION()
  void HandleActionHover();
  UFUNCTION()
  void HandleActionUnhover();
  UFUNCTION()
  void HandleSliderChange(float Value);
  UFUNCTION()
  void HandleSliderRelease();
};