#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "LibraryEntryWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class OSC3_API ULibraryEntryWidget : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;	
  virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

  UPROPERTY(meta = (BindWidget))
	UTextBlock* PluginName;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* ModuleName;

  UPROPERTY(meta = (BindWidget))
	UButton* Button;
private:
  UFUNCTION()
  void PrintSlug();

  FString PluginSlug, ModuleSlug;
};