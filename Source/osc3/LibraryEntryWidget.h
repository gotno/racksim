#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "LibraryEntryWidget.generated.h"

class Aosc3GameModeBase;
class UTextBlock;
class UCommonTextBlock;
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
  UTextBlock* TagsText;
  UPROPERTY(meta = (BindWidget))
  UCommonTextBlock* DescriptionText;

  UPROPERTY(meta = (BindWidget))
  UButton* Button;

  UPROPERTY(meta = (BindWidget))
  UButton* FavoriteToggleButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* FavoriteToggleButtonLabel;
private:
  UFUNCTION()
  void RequestModuleSpawn();
  UFUNCTION()
  void HandleButtonHover();
  UFUNCTION()
  void HandleButtonUnhover();
  UFUNCTION()
  void HandleFavoriteHover();
  UFUNCTION()
  void HandleFavoriteUnhover();
  UFUNCTION()
  void HandleFavoriteClick();

  FString PluginSlug, ModuleSlug;
  bool bFavorite{false};
  
  FSlateColor HoverColor{FLinearColor(0.40625f, 0.217116f, 0.282548f)};
  FSlateColor DefaultColor{FLinearColor(0.083333f, 0.083333f, 0.083333f)};
  
  Aosc3GameModeBase* GameMode;
};