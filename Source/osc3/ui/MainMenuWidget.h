#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class Aosc3GameState;
class UBorder;
class UButton;
class UListView;
class UFileListEntryData;

UCLASS()
class OSC3_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
  void UpdateState(Aosc3GameState* GameState);
  void GotoLoading();
  void GotoMain();

  void SetExitFunction(TFunction<void ()> inExitFunction) {
    ExitFunction = inExitFunction;
  }
  void SetSaveFunction(TFunction<void ()> inSaveFunction) {
    SaveFunction = inSaveFunction;
  }
  void SetNewFunction(TFunction<void ()> inNewFunction) {
    NewFunction = inNewFunction;
  }
  void SetContinueFunction(TFunction<void ()> inContinueFunction) {
    ContinueFunction = inContinueFunction;
  }
  void SetLoadFunction(TFunction<void (FString)> inLoadFunction) {
    LoadFunction = inLoadFunction;
  }
  void SetRecentPatchesListItems(TArray<UFileListEntryData*> Entries);
	
protected:
	virtual void NativeConstruct() override;	

  UPROPERTY(meta = (BindWidget))
  UBorder* MainSection;
  UPROPERTY(meta = (BindWidget))
  UButton* ContinueButton;
  UPROPERTY(meta = (BindWidget))
  UButton* SaveButton;
  // UPROPERTY(meta = (BindWidget))
  // UButton* SaveAsButton;
  UPROPERTY(meta = (BindWidget))
  UButton* NewButton;
  UPROPERTY(meta = (BindWidget))
  UButton* LoadButton;
  UPROPERTY(meta = (BindWidget))
  UButton* ExitButton;

  UPROPERTY(meta = (BindWidget))
	UListView* RecentPatchesList;

  UPROPERTY(meta = (BindWidget))
  UBorder* LoadingSection;
  
private:
  void HideAll();

  TFunction<void ()> ExitFunction;
  UFUNCTION()
  void HandleExitClick() { ExitFunction(); }

  TFunction<void ()> SaveFunction;
  UFUNCTION()
  void HandleSaveClick() {
    GotoLoading();
    SaveFunction();
  }

  TFunction<void ()> NewFunction;
  UFUNCTION()
  void HandleNewClick() {
    GotoLoading();
    NewFunction();
  }

  TFunction<void ()> ContinueFunction;
  UFUNCTION()
  void HandleContinueClick() {
    GotoLoading();
    ContinueFunction();
  }

  TFunction<void (FString)> LoadFunction;
};
