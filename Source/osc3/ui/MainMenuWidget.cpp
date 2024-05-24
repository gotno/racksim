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
#include "Components/HorizontalBox.h"
#include "Components/VerticalBoxSlot.h"

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();
  
  ConfirmationConfirmButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleConfirmationConfirmClick);
  ConfirmationCancelButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleConfirmationCancelClick);

  ExitButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleExitClick);
  SaveButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveClick);
  SaveAsButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveAsClick);
  NewButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleNewClick);
  ContinueButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleContinueClick);
  OverwriteTemplateButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleOverwriteTemplateClick);
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

  // Overwrite template button
  OverwriteTemplateButtonContainer->SetVisibility(
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

  bPatchIsSaved = !GameState->IsUnsaved();
}

void UMainMenuWidget::GotoLoading(FString UpperText, FString LowerText) {
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

void UMainMenuWidget::SetKeyboardInputText(FString Text) {
  FilenameInput->SetText(FText::FromString(Text));
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
    GotoLoading(FPaths::GetCleanFilename(PatchPath), LoadingPatchLabel);
    LoadFunction(PatchPath);
  } else {
    Confirm(
      TEXT("The current patch is unsaved!\nAre you sure you want to open this one?"),
      TEXT("Yes, open patch"),
      [this, PatchPath]() {
        GotoLoading(FPaths::GetCleanFilename(PatchPath), LoadingPatchLabel);
        LoadFunction(PatchPath);
      }
    );
  }
}

void UMainMenuWidget::HandleNewClick() {
  if (bPatchIsSaved) {
    GotoLoading(TEXT(""), LoadingPatchLabel);
    NewFunction();
  } else {
    Confirm(
      TEXT("The current patch is unsaved!\nAre you sure you want to create a new one?"),
      TEXT("Yes, create patch"),
      [this]() {
        GotoLoading(TEXT(""), LoadingPatchLabel);
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
      GotoLoading(TEXT(""), TEXT("overwriting template"));
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
  // Confirm(ConfirmationLabel, ConfirmButtonLabel, inConfirmationConfirmFunction, bSolo);
}