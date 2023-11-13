#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "BasicListEntryWidget.generated.h"

class Aosc3GameModeBase;
class UBasicListEntryData;

class UTextBlock;
class UButton;

UCLASS()
class OSC3_API UBasicListEntryWidget : public UUserWidget, public IUserObjectListEntry {
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
  UBasicListEntryData* EntryData;
  Aosc3GameModeBase* GameMode;
  
  bool bSelected{false};
  void SetSelected(bool inSelected);

  UFUNCTION()
  void HandleClick();
};
