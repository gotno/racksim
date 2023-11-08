#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "LibraryEntryWidget.generated.h"

class Aosc3GameModeBase;
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
	UTextBlock* Tags;

  UPROPERTY(meta = (BindWidget))
	UButton* Button;
private:
  UFUNCTION()
  void RequestModuleSpawn();

  FString PluginSlug, ModuleSlug;
  
  Aosc3GameModeBase* GameMode;
};