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
class UHorizontalBox;

UCLASS()
class OSC3_API UMainMenuWidget : public UUserWidget
{
  GENERATED_BODY()
  friend class AMainMenu;

public:
  UFUNCTION()
  void GotoMain();

  void GotoStatus(FString UpperText = "", FString LowerText = "");
  void UpdateState(Aosc3GameState* GameState);

  void SetExitFunction(TFunction<void ()> inExitFunction) {
    ExitFunction = inExitFunction;
  }
  void SetSaveFunction(TFunction<void ()> inSaveFunction) {
    SaveFunction = inSaveFunction;
  }
  void SetSaveAsFunction(TFunction<void (FString)> inSaveAsFunction) {
    SaveAsFunction = inSaveAsFunction;
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
  void SetOverwriteTemplateFunction(TFunction<void ()> inOverwriteTemplateFunction) {
    OverwriteTemplateFunction = inOverwriteTemplateFunction;
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
  void SetEnvironmentLightIntensityUpdateFunction(TFunction<void (float)> inEnvironmentLightIntensityUpdateFunction) {
    EnvironmentLightIntensityUpdateFunction = inEnvironmentLightIntensityUpdateFunction;
  }
  void SetEnvironmentLightAngleUpdateFunction(TFunction<void (float)> inEnvironmentLightAngleUpdateFunction) {
    EnvironmentLightAngleUpdateFunction = inEnvironmentLightAngleUpdateFunction;
  }
  void SetLoadMapFunction(TFunction<void (FString)> inLoadMapFunction) {
    LoadMapFunction = inLoadMapFunction;
  }

  void SetKeyboard(AKeyboard* inKeyboard);

  void Confirm(
    FString ConfirmationLabel,
    FString ConfirmButtonLabel,
    TFunction<void ()> inConfirmationConfirmFunction,
    bool bSolo = false
  );
  void Confirm(
    FString ConfirmationLabel,
    FString ConfirmButtonLabel,
    TFunction<void ()> inConfirmationConfirmFunction,
    TFunction<void ()> inConfirmationCancelFunction,
    bool bSolo = false
  );

protected:
  virtual void NativeConstruct() override;	

  // title
  UPROPERTY(meta = (BindWidget))
  UHorizontalBox* TitleContainer;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* Title;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* TitlePatchPath;

  // confirmation
  UPROPERTY(meta = (BindWidget))
  UBorder* ConfirmationSection;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* ConfirmationText;
  UPROPERTY(meta = (BindWidget))
  UButton* ConfirmationConfirmButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* ConfirmationConfirmButtonLabel;
  UPROPERTY(meta = (BindWidget))
  UButton* ConfirmationCancelButton;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* ConfirmationCancelButtonLabel;

  // file section
  UPROPERTY(meta = (BindWidget))
  UBorder* MainSection;
  UPROPERTY(meta = (BindWidget))
  UButton* ContinueButton;
  UPROPERTY(meta = (BindWidget))
  UButton* NewButton;
  UPROPERTY(meta = (BindWidget))
  UButton* LoadButton;
  UPROPERTY(meta = (BindWidget))
  USizeBox* SaveButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* SaveButton;
  UPROPERTY(meta = (BindWidget))
  USizeBox* SaveAsButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* SaveAsButton;
  UPROPERTY(meta = (BindWidget))
  USizeBox* OverwriteTemplateButtonContainer;
  UPROPERTY(meta = (BindWidget))
  UButton* OverwriteTemplateButton;
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
  UButton* MapOneButton;
  UPROPERTY(meta = (BindWidget))
  UButton* MapTwoButton;
  UPROPERTY(meta = (BindWidget))
  UButton* MapThreeButton;
  UPROPERTY(meta = (BindWidget))
  USlider* EnvironmentLightIntensitySlider;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* EnvironmentLightIntensitySliderLabel;
  UPROPERTY(meta = (BindWidget))
  USlider* EnvironmentLightAngleSlider;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* EnvironmentLightAngleSliderLabel;

  UPROPERTY(meta = (BindWidget))
  UBorder* LoadingSection;
  // UBorder* StatusSection; // aactually
  UPROPERTY(meta = (BindWidget))
  UTextBlock* UpperLoadingText;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* LowerLoadingText;

  UPROPERTY(meta = (BindWidget))
  UBorder* FilenameInputContainer;
  UPROPERTY(meta = (BindWidget))
  UTextBlock* FilenameInput;
  
private:
  AKeyboard* Keyboard;

  UFUNCTION()
  void HandleKeyboardInputUpdated(FString Input, int8 CursorIndex);
  UFUNCTION()
  void HandleKeyboardInputConfirmed(FString Input);

  FString CurrentFMDirectory{""};

  void HideAll();

  TFunction<void ()> ConfirmationConfirmFunction;
  UFUNCTION()
  void HandleConfirmationConfirmClick();

  TFunction<void ()> ConfirmationCancelFunction;
  UFUNCTION()
  void HandleConfirmationCancelClick();

  FString LoadingPatchLabel = LOADING_PATCH_LABEL;

  TFunction<void ()> ExitFunction;
  UFUNCTION()
  void HandleExitClick() {
    GotoStatus("goodbye", "leaving simulation");
    ExitFunction();
  }

  TFunction<void ()> SaveFunction;
  UFUNCTION()
  void HandleSaveClick() {
    GotoStatus();
    SaveFunction();
  }

  bool bSavingAs{false};
  UFUNCTION()
  void HandleSaveAsClick();
  TFunction<void (FString)> SaveAsFunction;

  FString LoadedPatchDirectory{""};
  FString LoadedPatchBasename{""};

  TFunction<void ()> NewFunction;
  UFUNCTION()
  void HandleNewClick();

  TFunction<void ()> ContinueFunction;
  UFUNCTION()
  void HandleContinueClick() {
    GotoStatus(TEXT(""), LoadingPatchLabel);
    ContinueFunction();
  }

  TFunction<void ()> OverwriteTemplateFunction;
  UFUNCTION()
  void HandleOverwriteTemplateClick();

  TFunction<void (FString)> LoadFunction;
  bool bPatchIsSaved{false};
  void HandleLoadPatch(FString PatchPath);

  UFUNCTION()
  void GotoFileManager();
  void LoadDirectoryInFileManager(FString Directory);
  void ReloadDirectoryInFileManager();
  UFileListEntryData* CreateListEntryData(
    FString Label,
    FString Path,
    EFileType Type,
    TFunction<void (FString)> ClickCallback,
    EFileIcon Icon = EFileIcon::None
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

  UFUNCTION()
  void HandleEnvironmentLightIntensitySliderChange(float Value);
  TFunction<void (float)> EnvironmentLightIntensityUpdateFunction;
  void SetEnvironmentLightIntensitySliderLabel(float& Value);

  UFUNCTION()
  void HandleEnvironmentLightAngleSliderChange(float Value);
  TFunction<void (float)> EnvironmentLightAngleUpdateFunction;
  void SetEnvironmentLightAngleSliderLabel(float& Value);

  TFunction<void (FString)> LoadMapFunction;
  UFUNCTION()
  void HandleMapOneClick() {
    GotoStatus(TEXT(""), "traveling to the light void");
    LoadMapFunction("light_void");
  }
  UFUNCTION()
  void HandleMapTwoClick() {
    GotoStatus(TEXT(""), "traveling to the dark void");
    LoadMapFunction("dark_void");
  }
  UFUNCTION()
  void HandleMapThreeClick() {
    GotoStatus(TEXT(""), "traveling to the park");
    LoadMapFunction("park");
  }
};
