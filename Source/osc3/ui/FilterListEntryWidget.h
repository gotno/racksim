#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "FilterListEntryWidget.generated.h"

class Aosc3GameModeBase;
class UFilterListEntryData;

class UTextBlock;
class UButton;

UCLASS()
class OSC3_API UFilterListEntryWidget : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;	
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
	UTextBlock* Label;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedIndicator;
  UPROPERTY(meta = (BindWidget))
	UButton* Button;
private:
  UFilterListEntryData* EntryData;
  Aosc3GameModeBase* GameMode;
  
  bool bSelected{false};
  void SetSelected(bool inSelected);

  UFUNCTION()
  void HandleClick();
};
