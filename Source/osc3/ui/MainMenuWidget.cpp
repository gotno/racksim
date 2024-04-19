#include "UI/MainMenuWidget.h"

#include "osc3GameState.h"
#include "UI/FileListEntryData.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();
  
  ExitButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleExitClick);
  SaveButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleSaveClick);
  NewButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleNewClick);
  ContinueButton->OnReleased.AddDynamic(this, &UMainMenuWidget::HandleContinueClick);
  ConfigurationButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoConfiguration);
  LoadButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoFileManager);
  FileManagerCancelButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoMain);

  CableOpacitySlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleCableOpacitySliderChange);
  CableOpacitySlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableOpacitySliderRelease);
  CableTensionSlider->OnValueChanged.AddDynamic(this, &UMainMenuWidget::HandleCableTensionSliderChange);
  CableTensionSlider->OnMouseCaptureEnd.AddDynamic(this, &UMainMenuWidget::HandleCableTensionSliderRelease);
  ConfigurationBackButton->OnReleased.AddDynamic(this, &UMainMenuWidget::GotoMain);
}

void UMainMenuWidget::UpdateState(Aosc3GameState* GameState) {
  // Continue with Autosave button
  ContinueButton->SetVisibility(
    !GameState->IsPatchLoaded() && GameState->CanContinueAutosave()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );

  // Save button
  SaveButton->SetVisibility(
    GameState->IsPatchLoaded()
      ? ESlateVisibility::Visible
      : ESlateVisibility::Collapsed
  );
  SaveButton->SetIsEnabled(GameState->IsUnsaved() && GameState->HasSaveFile());
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

void UMainMenuWidget::GotoConfiguration() {
  HideAll();
  ConfigurationSection->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::HideAll() {
  MainSection->SetVisibility(ESlateVisibility::Hidden);
  FileManagerSection->SetVisibility(ESlateVisibility::Hidden);
  LoadingSection->SetVisibility(ESlateVisibility::Hidden);
  ConfigurationSection->SetVisibility(ESlateVisibility::Hidden);
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
      LoadFunction(PatchPath);
      GotoLoading();
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

void UMainMenuWidget::HandleCableOpacitySliderChange(float Value) {
  CableOpacitySliderLabel->SetText(
    FText::FromString(FString("Cable opacity: ") + FString::SanitizeFloat(Value * 100.f) + FString("%"))
  );
}

void UMainMenuWidget::HandleCableOpacitySliderRelease() {
  CableOpacityUpdateFunction(CableOpacitySlider->GetValue());
}

void UMainMenuWidget::HandleCableTensionSliderChange(float Value) {
  CableTensionSliderLabel->SetText(
    FText::FromString(FString("Cable tension: ") + FString::SanitizeFloat(Value * 100.f) + FString("%"))
  );
}

void UMainMenuWidget::HandleCableTensionSliderRelease() {
  CableTensionUpdateFunction(CableTensionSlider->GetValue());
}