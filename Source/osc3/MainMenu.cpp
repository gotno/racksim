#include "MainMenu.h"

#include "osc3GameModeBase.h"
#include "osc3GameState.h"
#include "Player/VRAvatar.h"
#include "UI/MainMenuWidget.h"

#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"

AMainMenu::AMainMenu() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);

  MainMenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Main Menu Widget"));
  MainMenuWidgetComponent->SetWindowFocusable(false);
  MainMenuWidgetComponent->SetupAttachment(GetRootComponent());

  static ConstructorHelpers::FClassFinder<UMainMenuWidget>
    mainMenuWidgetObject(WidgetBlueprintReference);
  if (mainMenuWidgetObject.Succeeded()) {
    MainMenuWidgetComponent->SetWidgetClass(mainMenuWidgetObject.Class);
  }
}

void AMainMenu::BeginPlay() {
  Super::BeginPlay();
	
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
	
  MainMenuWidget = Cast<UMainMenuWidget>(MainMenuWidgetComponent->GetUserWidgetObject());
  
  FVector2D drawSize(933.f, 700.f);
  float desiredWorldHeight = 30.f;
  float scale = desiredWorldHeight / drawSize.Y;

  MainMenuWidgetComponent->SetDrawSize(drawSize);
  MainMenuWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));
}

void AMainMenu::Init(
  AVRAvatar* inPlayerPawn,
  Aosc3GameState* inGameState,
  TFunction<void ()> ExitFunction,
  TFunction<void ()> NewFunction,
  TFunction<void ()> ContinueFunction
) {
  PlayerPawn = inPlayerPawn;
  GameState = inGameState;
  MainMenuWidget->SetExitFunction(ExitFunction);
  MainMenuWidget->SetNewFunction(NewFunction);
  MainMenuWidget->SetContinueFunction(ContinueFunction);
  MainMenuWidget->UpdateState(GameState);
}

void AMainMenu::Hide() {
  MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
  SetActorHiddenInGame(true);
}

void AMainMenu::Show() {
  MainMenuWidget->GotoMain();
  MainMenuWidget->UpdateState(GameState);
  MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
  SetActorHiddenInGame(false);
}

void AMainMenu::Toggle() {
  if (!GameState->IsPatchLoaded()) return;

  if (IsHidden()) Show();
  else Hide();
}

void AMainMenu::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (PlayerPawn) {
    SetActorRotation(PlayerPawn->GetLookAtCameraRotation(GetActorLocation()));
    SetActorLocation(
      FMath::VInterpTo(
        GetActorLocation(),
        PlayerPawn->GetMainMenuPosition(),
        DeltaTime,
        10.f
      )
    );
  }
}

