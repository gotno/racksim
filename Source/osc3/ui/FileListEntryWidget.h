#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "FileListEntryWidget.generated.h"

class Aosc3GameModeBase;
class UFileListEntryData;

class UTextBlock;
class UCommonTextBlock;
class UButton;

UCLASS()
class OSC3_API UFileListEntryWidget : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;	
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
  UCommonTextBlock* Label;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* LabelStill;
  UPROPERTY(meta = (BindWidget))
  UButton* Button;
private:
  UFileListEntryData* EntryData;

  UFUNCTION()
  void HandleClick();

  UFUNCTION()
  void HandleHover();
  UFUNCTION()
  void HandleUnhover();
};
