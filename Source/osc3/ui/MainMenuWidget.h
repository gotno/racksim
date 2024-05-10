#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "osc3.h"

#include "MainMenuWidget.generated.h"

class Aosc3GameState;
class AKeyboard;
class UBorder;
class UButton;
class UListView;
class UFileListEntryData;
class UTextBlock;
class USlider;
class UCheckBox;
class USizeBox;

UCLASS()
class OSC3_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

  friend class AMainMenu;

public:
  UFUNCTION()
  void GotoMain();

  void UpdateState(Aosc3GameState* GameState);
  void GotoLoading();

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
  void SetFMDrivesListItems(TArray<UFileListEntryData*> Entries);
  void SetFMShortcutsListItems(TArray<UFileListEntryData*> Entries);

  void SetCableOpacityUpdateFunction(TFunction<void (float)> inCableOpacityUpdateFunction) {
    CableOpacityUpdateFunction = inCableOpacityUpdateFunction;
  }
  void SetCableTensionUpdateFunction(TFunction<void (float)> inCableTensionUpdateFunction) {
    CableTensionUpdateFunction = inCableTensionUpdateFunction;
  }
  void SetCableColorCycleToggleFunction(TFunction<void (bool)> inCableColorCycleToggleFunction) {
    CableColorCycleToggleFunction = inCableColorCycleToggleFunction;
  }

  void SetKeyboardInputText(FString Text);
	
protected:
	virtual void NativeConstruct() override;	

  // file section
  UPROPERTY(meta = (BindWidget))
  UBorder* MainSection;
  UPROPERTY(meta = (BindWidget))
  UButton* ContinueButton;
  UPROPERTY(meta = (BindWidget))
  USizeBox* SaveButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* SaveButton;
  UPROPERTY(meta = (BindWidget))
  USizeBox* SaveAsButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* SaveAsButton;
  UPROPERTY(meta = (BindWidget))
  UButton* NewButton;
  UPROPERTY(meta = (BindWidget))
  UButton* LoadButton;
  UPROPERTY(meta = (BindWidget))
  UButton* ExitButton;

  UPROPERTY(meta = (BindWidget))
	UListView* RecentPatchesList;

  // filebrowser section
  // TODO: rename this stuff FileBrowser*
  UPROPERTY(meta = (BindWidget))
  UBorder* FileManagerSection;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* FileListHeadingText;
  UPROPERTY(meta = (BindWidget))
	UListView* FileDrivesList;
  UPROPERTY(meta = (BindWidget))
	UListView* FileShortcutsList;
  UPROPERTY(meta = (BindWidget))
	UListView* FileBrowserList;
  UPROPERTY(meta = (BindWidget))
  UButton* FileManagerCancelButton;

  // config section
  UPROPERTY(meta = (BindWidget))
  USlider* CableOpacitySlider;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* CableOpacitySliderLabel;
  UPROPERTY(meta = (BindWidget))
  USlider* CableTensionSlider;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* CableTensionSliderLabel;
  UPROPERTY(meta = (BindWidget))
  UCheckBox* CableColorCycleToggle;

  UPROPERTY(meta = (BindWidget))
  UBorder* LoadingSection;
  // UBorder* StatusSection; // aactually

  UPROPERTY(meta = (BindWidget))
  UBorder* FilenameInputContainer;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* FilenameInput;
  
private:
  AKeyboard* Keyboard;

  FString CurrentFMDirectory{""};

  void HideAll();

  TFunction<void ()> ExitFunction;
  UFUNCTION()
  void HandleExitClick() {
    GotoLoading();
    ExitFunction();
  }

  TFunction<void ()> SaveFunction;
  UFUNCTION()
  void HandleSaveClick() {
    GotoLoading();
    SaveFunction();
  }

  bool bSavingAs{false};
  UFUNCTION()
  void HandleSaveAsClick();

  FString LoadedPatchDirectory{""};
  FString LoadedPatchBasename{""};

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

  UFUNCTION()
  void GotoFileManager();
  void LoadDirectoryInFileManager(FString Directory);
  void ReloadDirectoryInFileManager();
  UFileListEntryData* CreateListEntryData(
    FString Label,
    FString Path,
    EFileType Type,
    TFunction<void (FString)> ClickCallback
  );
  void SetFileListHeadingText(FString HeadingText);

  UFUNCTION()
  void HandleCableOpacitySliderChange(float Value);
  UFUNCTION()
  void HandleCableOpacitySliderRelease();
  TFunction<void (float)> CableOpacityUpdateFunction;
  UFUNCTION()
  void HandleCableTensionSliderChange(float Value);
  UFUNCTION()
  void HandleCableTensionSliderRelease();
  TFunction<void (float)> CableTensionUpdateFunction;
  UFUNCTION()
  void HandleCableColorCycleToggle(bool bIsChecked);
  TFunction<void (bool)> CableColorCycleToggleFunction;
};
