#include "UI/MainMenuWidget.h"

#include "osc3GameState.h"
#include "RackSimGameUserSettings.h"

#include "UI/FileListEntryData.h"
#include "UI/Keyboard.h"
#include "UI/MultiToggleButton.h"
#include "UI/LabeledSlider.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBoxSlot.h"

void UMainMenuWidget::SynchronizeProperties() {
  Super::SynchronizeProperties();

  if (CableColorCycleToggleButton)
    CableColorCycleToggleButton->SetPrimaryLabel(TEXT("Auto-cycle cable color"));
  if (CableOpacitySlider) {
    CableOpacitySlider->SetLabel(TEXT("Cable opacity"));
  }
  if (CableTensionSlider) {
    CableTensionSlider->SetLabel(TEXT("Cable Tension"));
  }

  if (ControllerTooltipsToggleButton) {
    ControllerTooltipsToggleButton->SetPrimaryLabel(TEXT("Show controller tooltips"));
    ControllerTooltipsToggleButton->SetToggleLabels(TEXT("L"), TEXT("R"));
  }
  if (ControllerLightsToggleButton) {
    ControllerLightsToggleButton->SetPrimaryLabel(TEXT("Show controller lights"));
    ControllerLightsToggleButton->SetToggleLabels(TEXT("L"), TEXT("R"));
  }

  if (EnvironmentLightIntensitySlider) {
    EnvironmentLightIntensitySlider->SetLabel(TEXT("Room Brightness"));
    EnvironmentLightIntensitySlider->SetValue(DEFAULT_ROOM_BRIGHTNESS_AMOUNT);
  }

  if (EnvironmentLightAngleSlider) {
    EnvironmentLightAngleSlider->SetLabel(TEXT("Sun Angle"));
    EnvironmentLightAngleSlider->SetMinValue(MIN_SUN_ANGLE);
    EnvironmentLightAngleSlider->SetMaxValue(MAX_SUN_ANGLE);
    EnvironmentLightAngleSlider->SetValue(DEFAULT_SUN_ANGLE_AMOUNT);
    EnvironmentLightAngleSlider->SetValueMultiplier(1.f);
    EnvironmentLightAngleSlider->SetUnit(TEXT(" degrees"));
  }
}

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();

  ConfirmationConfirmButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleConfirmationConfirmClick);
  ConfirmationCancelButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleConfirmationCancelClick);

  CableColorCycleToggleButton->OnToggleOneToggledDelegate.AddDynamic(
    this,
    &UMainMenuWidget::HandleCableColorCycleToggle
  );

  ControllerLightsToggleButton->OnToggleOneToggledDelegate.AddDynamic(
    this,
    &UMainMenuWidget::HandleControllerLightToggleLeft
  );
  ControllerLightsToggleButton->OnToggleTwoToggledDelegate.AddDynamic(
    this,
    &UMainMenuWidget::HandleControllerLightToggleRight
  );

  ControllerTooltipsToggleButton->OnToggleOneToggledDelegate.AddDynamic(
    this,
    &UMainMenuWidget::HandleControllerTooltipToggleLeft
  );
  ControllerTooltipsToggleButton->OnToggleTwoToggledDelegate.AddDynamic(
    this,
    &UMainMenuWidget::HandleControllerTooltipToggleRight
  );

  ExitButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleExitClick);
  SaveButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveClick);
  SaveAsButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveAsClick);
  NewButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleNewClick);
  ContinueButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleContinueClick);
  OverwriteTemplateButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleOverwriteTemplateClick);
  LoadButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoFileManager);
  FileManagerCancelButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoMain);

  CableOpacitySlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableOpacitySliderRelease);
  CableTensionSlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableTensionSliderRelease);

  EnvironmentLightIntensitySlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleEnvironmentLightIntensitySliderChange);
  EnvironmentLightAngleSlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleEnvironmentLightAngleSliderChange);

  MapOneButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleMapOneClick);
  MapTwoButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleMapTwoClick);
  MapThreeButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleMapThreeClick);
}

void UMainMenuWidget::UpdateSettings(URackSimGameUserSettings* UserSettings) {
  CableTensionSlider->SetValue(UserSettings->CableTension);
  CableOpacitySlider->SetValue(UserSettings->CableOpacity);

  CableColorCycleToggleButton->SetToggleOneChecked(UserSettings->bCycleCableColors);

  ControllerLightsToggleButton->SetToggleOneChecked(UserSettings->bShowLeftControllerLight);
  ControllerLightsToggleButton->SetToggleTwoChecked(UserSettings->bShowRightControllerLight);

  ControllerTooltipsToggleButton->SetToggleOneChecked(UserSettings->bShowLeftControllerTooltip);
  ControllerTooltipsToggleButton->SetToggleTwoChecked(UserSettings->bShowRightControllerTooltip);
}

void UMainMenuWidget::UpdateState(Aosc3GameState* GameState) {
  Cast<UVerticalBoxSlot>(TitleContainer->Slot)->SetHorizontalAlignment(
    GameState->IsPatchLoaded() ? EHorizontalAlignment::HAlign_Left : EHorizontalAlignment::HAlign_Center
  );
  TitlePatchPath->SetVisibility(
    GameState->IsPatchLoaded() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
  );

  if (GameState->GetPatchPath().IsEmpty()) {
    TitlePatchPath->SetText(FText::FromString(FString("*[unsaved patch]")));
  } else {
    FString filename = FPaths::GetBaseFilename(GameState->GetPatchPath(), true);
    filename.Append(".vcv");
    if (GameState->IsUnsaved()) filename.InsertAt(0, "*");
    TitlePatchPath->SetText(FText::FromString(filename));
  }

  // Continue with Autosave button
  ContinueButton->SetVisibility(
    GameState->CanContinueAutosave()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  ContinueButtonLabel->SetText(
    GameState->IsPatchLoaded()
      ? FText::FromString("Reload Autosave")
      : FText::FromString("Continue with Autosave")
  );

  // Save button
  SaveButtonContainer->SetVisibility(
    GameState->IsPatchLoaded()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  SaveButton->SetIsEnabled(GameState->IsUnsaved() && GameState->HasSaveFile());

  // Save As button
  SaveAsButtonContainer->SetVisibility(
    GameState->IsPatchLoaded()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );

  // Overwrite template button
  OverwriteTemplateButtonContainer->SetVisibility(
    GameState->IsPatchLoaded()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );

  // patch file info
  FString patchPath = GameState->GetPatchPath();
  if (patchPath.IsEmpty()) {
    LoadedPatchDirectory = "";
    LoadedPatchBasename = "";
  } else {
    LoadedPatchDirectory = FPaths::GetPath(patchPath);
    LoadedPatchDirectory.Append("\\");
    LoadedPatchBasename = FPaths::GetBaseFilename(patchPath, true);
  }
  bPatchIsSaved = !GameState->IsUnsaved();

  // lighting controls
  if (GameState->CurrentMapName.Equals("park")) {
    EnvironmentLightAngleSlider->SetVisibility(ESlateVisibility::Visible);
  } else {
    EnvironmentLightAngleSlider->SetVisibility(ESlateVisibility::Collapsed);
  }
  EnvironmentLightIntensitySlider->SetValue(GameState->EnvironmentLightIntensityAmount);
  EnvironmentLightAngleSlider->SetValue(GameState->EnvironmentLightAngleAmount);

  // map load buttons
  MapOneButton->SetIsEnabled(!GameState->CurrentMapName.Equals("light_void"));
  MapTwoButton->SetIsEnabled(!GameState->CurrentMapName.Equals("dark_void"));
  MapThreeButton->SetIsEnabled(!GameState->CurrentMapName.Equals("park"));
}

void UMainMenuWidget::SetKeyboard(AKeyboard* inKeyboard) {
  Keyboard = inKeyboard;
  Keyboard->AddOnInputUpdatedDelegate(this, FName("HandleKeyboardInputUpdated"));
  Keyboard->AddOnInputConfirmedDelegate(this, FName("HandleKeyboardInputConfirmed"));
}

void UMainMenuWidget::HandleKeyboardInputUpdated(FString Input, int8 CursorIndex) {
  Input.InsertAt(CursorIndex, "|");
  Input.Append(".vcv");
  FilenameInput->SetText(FText::FromString(Input));
}

void UMainMenuWidget::HandleKeyboardInputConfirmed(FString Input) {
  FString path = CurrentFMDirectory;
  path.Append(Input);
  path.Append(".vcv");

  if (FPaths::FileExists(path)) {
    Confirm(
      TEXT("File exists! Overwrite?"),
      TEXT("Yes, overwrite file"),
      [this, path]() {
        GotoStatus(TEXT(""), TEXT("saving patch"));
        SaveAsFunction(path);
      }
    );
  } else {
    GotoStatus(TEXT(""), TEXT("saving patch"));
    SaveAsFunction(path);
  }
}

void UMainMenuWidget::GotoStatus(FString UpperText, FString LowerText) {
  HideAll();
  LoadingSection->SetVisibility(ESlateVisibility::HitTestInvisible);
  UpperLoadingText->SetText(FText::FromString(UpperText));
  LowerLoadingText->SetText(FText::FromString(LowerText));
}

void UMainMenuWidget::GotoMain() {
  HideAll();
  MainSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::GotoFileManager() {
  HideAll();
  FileManagerSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::HideAll() {
  MainSection->SetVisibility(ESlateVisibility::Hidden);
  FileManagerSection->SetVisibility(ESlateVisibility::Hidden);
  LoadingSection->SetVisibility(ESlateVisibility::Hidden);
  ConfirmationSection->SetVisibility(ESlateVisibility::Hidden);

  // keyboard/input
  bSavingAs = false;
  ReloadDirectoryInFileManager();
  FilenameInputContainer->SetVisibility(ESlateVisibility::Collapsed);
  Keyboard->SetActorHiddenInGame(true);
}

void UMainMenuWidget::SetRecentPatchesListItems(TArray<UFileListEntryData*> Entries) {
  for (UFileListEntryData* entry : Entries) {
    entry->ClickCallback = [this](FString PatchPath) {
      HandleLoadPatch(PatchPath);
    };
  }
  RecentPatchesList->SetListItems(Entries);
}

void UMainMenuWidget::SetFMDrivesListItems(TArray<UFileListEntryData*> Entries) {
  for (UFileListEntryData* entry : Entries) {
    entry->ClickCallback = [this](FString Directory) {
      LoadDirectoryInFileManager(Directory);
    };
  }
  FileDrivesList->SetListItems(Entries);
}

void UMainMenuWidget::SetFMShortcutsListItems(TArray<UFileListEntryData*> Entries) {
  for (UFileListEntryData* entry : Entries) {
    entry->ClickCallback = [this](FString Directory) {
      LoadDirectoryInFileManager(Directory);
    };
  }
  FileShortcutsList->SetListItems(Entries);
}

UFileListEntryData* UMainMenuWidget::CreateListEntryData(FString Label, FString Path, EFileType Type, TFunction<void (FString)> ClickCallback, EFileIcon Icon) {
  UFileListEntryData* entry = NewObject<UFileListEntryData>(this);
  entry->Label = Label;
  entry->Path = Path;
  entry->Type = Type;
  entry->Icon = Icon;
  entry->ClickCallback = ClickCallback;
  return entry;
}

void UMainMenuWidget::SetFileListHeadingText(FString HeadingText) {
  HeadingText.ReplaceInline(TEXT("/"), TEXT("\\"), ESearchCase::CaseSensitive);
  FileListHeadingText->SetText(FText::FromString(HeadingText));
}

void UMainMenuWidget::LoadDirectoryInFileManager(FString Directory) {
  CurrentFMDirectory = Directory;
  SetFileListHeadingText(Directory);

  TArray<UFileListEntryData*> entries;

  IFileManager& FileManager = IFileManager::Get();

  FString parentDirectory = FPaths::GetPath(FPaths::GetPath(Directory)).Append("/");
  if (!parentDirectory.Equals("/")) {
    entries.Add(CreateListEntryData("..", parentDirectory, EFileType::Directory, [this](FString Directory) {
      LoadDirectoryInFileManager(Directory);
    }, EFileIcon::Directory));
  }

  TArray<FString> found;
  FileManager.FindFiles(found, *(Directory + "*"), false, true);
  for (FString& childDirectory : found) {
    entries.Add(CreateListEntryData(childDirectory, Directory + childDirectory + "/", EFileType::Directory, [this](FString Directory) {
      LoadDirectoryInFileManager(Directory);
    }, EFileIcon::Directory));
  }

  found.Empty();
  FileManager.FindFiles(found, *(Directory + "*.vcv"), true, false);
  for (FString& filename : found) {
    entries.Add(CreateListEntryData(filename, Directory + filename, EFileType::File, [this](FString PatchPath) {
      if (!bSavingAs) {
        HandleLoadPatch(PatchPath);
      } else {
        Keyboard->SetInput(FPaths::GetBaseFilename(PatchPath, true));
      }
    }, EFileIcon::File));
  }

  FileBrowserList->SetListItems(entries);
}

void UMainMenuWidget::ReloadDirectoryInFileManager() {
  LoadDirectoryInFileManager(CurrentFMDirectory);
}

void UMainMenuWidget::HandleCableOpacitySliderRelease() {
  CableOpacityUpdateFunction(CableOpacitySlider->GetValue());
}

void UMainMenuWidget::HandleCableTensionSliderRelease() {
  CableTensionUpdateFunction(CableTensionSlider->GetValue());
}

void UMainMenuWidget::HandleEnvironmentLightIntensitySliderChange(float Value) {
  EnvironmentLightIntensityUpdateFunction(Value);
}

void UMainMenuWidget::HandleEnvironmentLightAngleSliderChange(float Value) {
  EnvironmentLightAngleUpdateFunction(Value);
}

void UMainMenuWidget::HandleCableColorCycleToggle(bool bIsChecked) {
  CableColorCycleToggleFunction(bIsChecked);
}

void UMainMenuWidget::HandleControllerLightToggleLeft(bool bIsChecked) {
  ControllerLightToggleFunction(!bIsChecked, EControllerHand::Left);
}
void UMainMenuWidget::HandleControllerLightToggleRight(bool bIsChecked) {
  ControllerLightToggleFunction(!bIsChecked, EControllerHand::Right);
}

void UMainMenuWidget::HandleControllerTooltipToggleLeft(bool bIsChecked) {
  ControllerTooltipToggleFunction(!bIsChecked, EControllerHand::Left);
}
void UMainMenuWidget::HandleControllerTooltipToggleRight(bool bIsChecked) {
  ControllerTooltipToggleFunction(!bIsChecked, EControllerHand::Right);
}

void UMainMenuWidget::HandleSaveAsClick() {
  GotoFileManager();

  bSavingAs = true;
  if (LoadedPatchDirectory.IsEmpty()) {
    ReloadDirectoryInFileManager();
    Keyboard->SetInput("");
  } else {
    LoadDirectoryInFileManager(LoadedPatchDirectory);
    Keyboard->SetInput(LoadedPatchBasename);
  }

  FilenameInputContainer->SetVisibility(ESlateVisibility::Visible);
  Keyboard->SetActorHiddenInGame(false);
}

void UMainMenuWidget::HandleConfirmationConfirmClick() {
  ConfirmationSection->SetVisibility(ESlateVisibility::Hidden);
  ConfirmationConfirmFunction();
}

void UMainMenuWidget::HandleConfirmationCancelClick() {
  ConfirmationSection->SetVisibility(ESlateVisibility::Hidden);
  ConfirmationCancelFunction();
}

void UMainMenuWidget::HandleLoadPatch(FString PatchPath) {
  if (bPatchIsSaved) {
    LoadFunction(PatchPath);
  } else {
    Confirm(
      TEXT("The current patch is unsaved!\nAre you sure you want to open this one?"),
      TEXT("Yes, open patch"),
      [this, PatchPath]() {
        LoadFunction(PatchPath);
      }
    );
  }
}

void UMainMenuWidget::HandleNewClick() {
  if (bPatchIsSaved) {
    NewFunction();
  } else {
    Confirm(
      TEXT("The current patch is unsaved!\nAre you sure you want to create a new one?"),
      TEXT("Yes, create patch"),
      [this]() {
        NewFunction();
      }
    );
  }
}

void UMainMenuWidget::HandleOverwriteTemplateClick() {
  Confirm(
    TEXT("Overwrite template patch?"),
    TEXT("Yes, overwrite patch"),
    [this]() {
      GotoStatus(TEXT(""), TEXT("overwriting template"));
      OverwriteTemplateFunction();
    }
  );
}

void UMainMenuWidget::Confirm(
  FString ConfirmationLabel,
  FString ConfirmButtonLabel,
  TFunction<void ()> inConfirmationConfirmFunction,
  bool bSolo
) {
  if (bSolo) HideAll();
  ConfirmationText->SetText(FText::FromString(ConfirmationLabel));
  ConfirmationConfirmButtonLabel->SetText(FText::FromString(ConfirmButtonLabel));
  ConfirmationConfirmFunction = inConfirmationConfirmFunction;
  ConfirmationCancelFunction = [this]() {
    ConfirmationSection->SetVisibility(ESlateVisibility::Hidden);
  };
  ConfirmationSection->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainMenuWidget::Confirm(
  FString ConfirmationLabel,
  FString ConfirmButtonLabel,
  TFunction<void ()> inConfirmationConfirmFunction,
  TFunction<void ()> inConfirmationCancelFunction,
  bool bSolo
) {
  if (bSolo) HideAll();
  ConfirmationText->SetText(FText::FromString(ConfirmationLabel));
  ConfirmationConfirmButtonLabel->SetText(FText::FromString(ConfirmButtonLabel));
  ConfirmationConfirmFunction = inConfirmationConfirmFunction;
  ConfirmationCancelFunction = inConfirmationCancelFunction;
  ConfirmationSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::Alert(FString AlertLabel, FString ConfirmButtonLabel) {
  HideAll();
  ConfirmationText->SetText(FText::FromString(AlertLabel));
  ConfirmationConfirmButtonLabel->SetText(FText::FromString(ConfirmButtonLabel));
  ConfirmationCancelButtonContainer->SetVisibility(ESlateVisibility::Collapsed);
  ConfirmationConfirmFunction = [&]() {
    ConfirmationCancelButtonContainer->SetVisibility(ESlateVisibility::Visible);
    GotoMain();
  };
  ConfirmationSection->SetVisibility(ESlateVisibility::Visible);
}