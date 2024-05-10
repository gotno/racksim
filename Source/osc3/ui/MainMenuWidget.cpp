#include "UI/MainMenuWidget.h"

#include "osc3GameState.h"
#include "UI/FileListEntryData.h"
#include "UI/Keyboard.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/SizeBox.h"

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();
  
  ExitButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleExitClick);
  SaveButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveClick);
  SaveAsButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveAsClick);
  NewButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleNewClick);
  ContinueButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleContinueClick);
  LoadButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoFileManager);
  FileManagerCancelButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoMain);

  CableOpacitySlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleCableOpacitySliderChange);
  CableOpacitySlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableOpacitySliderRelease);
  // TODO: check for GameUserSettings
  CableOpacitySlider->SetValue(DEFAULT_CABLE_OPACITY);
  HandleCableOpacitySliderChange(DEFAULT_CABLE_OPACITY);

  CableTensionSlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleCableTensionSliderChange);
  CableTensionSlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableTensionSliderRelease);
  // TODO: check for GameUserSettings
  CableTensionSlider->SetValue(DEFAULT_CABLE_TENSION);
  HandleCableTensionSliderChange(DEFAULT_CABLE_TENSION);

  CableColorCycleToggle->OnCheckStateChanged.AddDynamic(this, &UMainMenuWidget::HandleCableColorCycleToggle);
}

void UMainMenuWidget::UpdateState(Aosc3GameState* GameState) {
  // Continue with Autosave button
  ContinueButton->SetVisibility(
    !GameState->IsPatchLoaded() && GameState->CanContinueAutosave()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
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

  FString patchPath = GameState->GetPatchPath();
  if (patchPath.IsEmpty()) {
    LoadedPatchDirectory = "";
    LoadedPatchBasename = "";
  } else {
    LoadedPatchDirectory = FPaths::GetPath(patchPath);
    LoadedPatchDirectory.Append("\\");
    LoadedPatchBasename = FPaths::GetBaseFilename(patchPath, true);
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

void UMainMenuWidget::GotoFileManager() {
  HideAll();
  FileManagerSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::HideAll() {
  MainSection->SetVisibility(ESlateVisibility::Hidden);
  FileManagerSection->SetVisibility(ESlateVisibility::Hidden);
  LoadingSection->SetVisibility(ESlateVisibility::Hidden);

  // keyboard/input
  bSavingAs = false;
  ReloadDirectoryInFileManager();
  FilenameInputContainer->SetVisibility(ESlateVisibility::Collapsed);
  Keyboard->SetActorHiddenInGame(true);
}

void UMainMenuWidget::SetKeyboardInputText(FString Text) {
  FilenameInput->SetText(FText::FromString(Text));
}

void UMainMenuWidget::SetRecentPatchesListItems(TArray<UFileListEntryData*> Entries) {
  for (UFileListEntryData* entry : Entries) {
    entry->ClickCallback = [this](FString PatchPath) {
      GotoLoading();
      LoadFunction(PatchPath);
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

UFileListEntryData* UMainMenuWidget::CreateListEntryData(FString Label, FString Path, EFileType Type, TFunction<void (FString)> ClickCallback) {
  UFileListEntryData* entry = NewObject<UFileListEntryData>(this);
  entry->Label = Label;
  entry->Path = Path;
  entry->Type = Type;
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
    }));
  }

  TArray<FString> found;
  FileManager.FindFiles(found, *(Directory + "*.vcv"), true, false);
  for (FString& filename : found) {
    entries.Add(CreateListEntryData(filename, Directory + filename, EFileType::File, [this](FString PatchPath) {
      if (!bSavingAs) {
        LoadFunction(PatchPath);
        GotoLoading();
      } else {
        Keyboard->SetInput(FPaths::GetBaseFilename(PatchPath, true));
      }
    }));
  }

  found.Empty();
  FileManager.FindFiles(found, *(Directory + "*"), false, true);
  for (FString& childDirectory : found) {
    entries.Add(CreateListEntryData(childDirectory, Directory + childDirectory + "/", EFileType::Directory, [this](FString Directory) {
      LoadDirectoryInFileManager(Directory);
    }));
  }

  FileBrowserList->SetListItems(entries);
  FPaths::NormalizeDirectoryName(Directory);
}

void UMainMenuWidget::ReloadDirectoryInFileManager() {
  LoadDirectoryInFileManager(CurrentFMDirectory);
}

void UMainMenuWidget::HandleCableOpacitySliderChange(float Value) {
  FString label{"Cable opacity: "};
  label.AppendInt(FMath::RoundToInt(Value * 100.f));
  label.Append("%");
  CableOpacitySliderLabel->SetText(FText::FromString(label));
}

void UMainMenuWidget::HandleCableOpacitySliderRelease() {
  CableOpacityUpdateFunction(CableOpacitySlider->GetValue());
}

void UMainMenuWidget::HandleCableTensionSliderChange(float Value) {
  FString label{"Cable tension: "};
  label.AppendInt(FMath::RoundToInt(Value * 100.f));
  label.Append("%");
  CableTensionSliderLabel->SetText(FText::FromString(label));
}

void UMainMenuWidget::HandleCableTensionSliderRelease() {
  CableTensionUpdateFunction(CableTensionSlider->GetValue());
}

void UMainMenuWidget::HandleCableColorCycleToggle(bool bIsChecked) {
  CableColorCycleToggleFunction(bIsChecked);
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