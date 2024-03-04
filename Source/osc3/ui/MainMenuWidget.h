#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class Aosc3GameState;
class UBorder;
class UButton;

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
  void SetNewFunction(TFunction<void ()> inNewFunction) {
    NewFunction = inNewFunction;
  }
  void SetContinueFunction(TFunction<void ()> inContinueFunction) {
    ContinueFunction = inContinueFunction;
  }
	
protected:
	virtual void NativeConstruct() override;	

  UPROPERTY(meta = (BindWidget))
  UBorder* MainSection;
  UPROPERTY(meta = (BindWidget))
  UButton* NewButton;
  UPROPERTY(meta = (BindWidget))
  UButton* ContinueButton;
  UPROPERTY(meta = (BindWidget))
  UButton* ExitButton;

  UPROPERTY(meta = (BindWidget))
  UBorder* LoadingSection;
  
private:
  void HideAll();

  TFunction<void ()> ExitFunction;
  UFUNCTION()
  void HandleExitClick() { ExitFunction(); }

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
};
