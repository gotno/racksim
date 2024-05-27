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

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;

  void Init(
    TFunction<void ()> ExitFunction,
    TFunction<void ()> SaveFunction,
    TFunction<void (FString)> SaveAsFunction,
    TFunction<void ()> NewFunction,
    TFunction<void ()> ContinueFunction,
    TFunction<void ()> OverwriteTemplateFunction,
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

  TArray<UFileListEntryData*> GenerateRecentPatchesEntries();
  TArray<UFileListEntryData*> GenerateFMDrivesEntries();
  TArray<UFileListEntryData*> GenerateFMShortcutsEntries();
  TArray<UFileListEntryData*> GenerateFMFileBrowserEntries(FString& Directory);
};
