#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "FileListEntryWidget.generated.h"

class Aosc3GameModeBase;
class UFileListEntryData;

class UTextBlock;
class UButton;

UCLASS()
class OSC3_API UFileListEntryWidget : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;	
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
	UTextBlock* Label;
  UPROPERTY(meta = (BindWidget))
	UButton* Button;
private:
  UFileListEntryData* EntryData;
  // Aosc3GameModeBase* GameMode;

  UFUNCTION()
  void HandleClick();
};
