#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainMenu.generated.h"

class Aosc3GameModeBase;
class Aosc3GameState;
class AVRAvatar;
class UFileListEntryData;

class UWidgetComponent;
class UMainMenuWidget;
class AKeyboard;

UCLASS()
class OSC3_API AMainMenu : public AActor {
	GENERATED_BODY()
	
public:	
	AMainMenu();

  UFUNCTION()
  void HandleKeyboardInputUpdated(FString Input, int8 CursorIndex);
  UFUNCTION()
  void HandleKeyboardInputConfirmed(FString Input);
  
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  void Init(
    TFunction<void ()> ExitFunction,
    TFunction<void ()> SaveFunction,
    TFunction<void (FString)> inSaveAsFunction,
    TFunction<void ()> NewFunction,
    TFunction<void ()> ContinueFunction,
    TFunction<void (FString)> LoadFunction,
    TFunction<void (float)> CableOpacityUpdateFunction,
    TFunction<void (float)> CableTensionUpdateFunction,
    TFunction<void (bool)> CableColorCycleToggleFunction
  );
  void Hide();
  void Show();
  void Refresh();
  void Toggle();

private:
  Aosc3GameModeBase* GameMode;
  Aosc3GameState* GameState;
  AVRAvatar* PlayerPawn;

  AKeyboard* Keyboard;

  USceneComponent* RootSceneComponent;

  UWidgetComponent* MainMenuWidgetComponent;
  TCHAR* WidgetBlueprintReference =
    TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_MainMenuWidget.BP_MainMenuWidget_C'");
  UMainMenuWidget* MainMenuWidget;

  TFunction<void (FString)> SaveAsFunction;

  TArray<UFileListEntryData*> GenerateRecentPatchesEntries();
  TArray<UFileListEntryData*> GenerateFMDrivesEntries();
  TArray<UFileListEntryData*> GenerateFMShortcutsEntries();
  TArray<UFileListEntryData*> GenerateFMFileBrowserEntries(FString& Directory);
};
