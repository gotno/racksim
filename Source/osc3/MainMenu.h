#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainMenu.generated.h"

class Aosc3GameModeBase;
class Aosc3GameState;
class AVRAvatar;

class UWidgetComponent;
class UMainMenuWidget;

UCLASS()
class OSC3_API AMainMenu : public AActor {
	GENERATED_BODY()
	
public:	
	AMainMenu();
  
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  void Init(
    TFunction<void ()> ExitFunction,
    TFunction<void ()> NewFunction,
    TFunction<void ()> ContinueFunction
  );
  void Hide();
  void Show();
  void Toggle();

private:
  Aosc3GameModeBase* GameMode;
  Aosc3GameState* GameState;
  AVRAvatar* PlayerPawn;

  USceneComponent* RootSceneComponent;

  UWidgetComponent* MainMenuWidgetComponent;
  TCHAR* WidgetBlueprintReference =
    TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_MainMenuWidget.BP_MainMenuWidget_C'");
  UMainMenuWidget* MainMenuWidget;
};
