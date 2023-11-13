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

  UPROPERTY(meta = (BindWidget))
	UButton* FavoriteToggleButton;
  UPROPERTY(meta = (BindWidget))
	UTextBlock* FavoriteToggleButtonLabel;
private:
  UFUNCTION()
  void RequestModuleSpawn();
  UFUNCTION()
  void HandleFavoriteHover();
  UFUNCTION()
  void HandleFavoriteUnhover();
  UFUNCTION()
  void HandleFavoriteClick();
  // UFUNCTION()
  // void SetFavorite(bool inFavorite);

  FString PluginSlug, ModuleSlug;
  bool bFavorite{false};
  
  FSlateColor HoverColor{FLinearColor(1.f, 0.09375f, 0.156451f)};
  FSlateColor DefaultColor{FLinearColor(1.f, 1.f, 1.f)};
  
  Aosc3GameModeBase* GameMode;
};