#include "UI/MainMenuWidget.h"

#include "osc3GameState.h"

#include "Components/Border.h"
#include "Components/Button.h"

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();
  
  ExitButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleExitClick);
  NewButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleNewClick);
  ContinueButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleContinueClick);
}

void UMainMenuWidget::UpdateState(Aosc3GameState* GameState) {
  if (GameState->IsPatchLoaded() || !GameState->CanContinueAutosave()) {
    ContinueButton->SetVisibility(ESlateVisibility::Collapsed);
  } else {
    ContinueButton->SetVisibility(ESlateVisibility::Visible);
  }
}

void UMainMenuWidget::GotoLoading() {
  HideAll();
  LoadingSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::GotoMain() {
  HideAll();
  MainSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::HideAll() {
  MainSection->SetVisibility(ESlateVisibility::Hidden);
  LoadingSection->SetVisibility(ESlateVisibility::Hidden);
}