#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "VCV.h"

#include "ContextMenuEntry.generated.h"

class Aosc3GameModeBase;
class AVCVModule;
class UButton;
class UTextBlock;
class UBorder;

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
	UTextBlock* ActionButtonText;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedIndicator;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SubmenuIndicator;

  UPROPERTY(meta = (BindWidget))
	UBorder* LabelContainer;
  UPROPERTY(meta = (BindWidget))
	UButton* Label;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* LabelText;

  UPROPERTY(meta = (BindWidget))
	UBorder* Range;

  UPROPERTY(meta = (BindWidget))
	UBorder* BackContainer;
  UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

  
private:
  Aosc3GameModeBase* GameMode;
  VCVMenuItem MenuItem;
  AVCVModule* Module;
  
  // for type: BACK
  int ParentMenuId;
  
  UFUNCTION()
  void HandleClick();
};