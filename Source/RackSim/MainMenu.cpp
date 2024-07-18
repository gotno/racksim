#include "MainMenu.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "osc3GameState.h"
#include "RackSimGameUserSettings.h"
#include "Player/VRAvatar.h"
#include "UI/MainMenuWidget.h"
#include "UI/FileListEntryData.h"
#include "UI/Keyboard.h"
#include "UI/KeyboardWidget.h"

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

  MainMenuWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  MainMenuWidgetComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  MainMenuWidgetComponent->SetCollisionResponseToChannel(WIDGET_TRACE, ECollisionResponse::ECR_Block);

  static ConstructorHelpers::FClassFinder<UMainMenuWidget>
    mainMenuWidgetObject(WidgetBlueprintReference);
  if (mainMenuWidgetObject.Succeeded()) {
    MainMenuWidgetComponent->SetWidgetClass(mainMenuWidgetObject.Class);
  }
}

void AMainMenu::BeginPlay() {
  Super::BeginPlay();
	
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  PlayerPawn = Cast<AVRAvatar>(UGameplayStatics::GetPlayerPawn(this, 0));
  GameState = Cast<Aosc3GameState>(UGameplayStatics::GetGameState(this));
	
  // set up menu widget
  MainMenuWidget = Cast<UMainMenuWidget>(MainMenuWidgetComponent->GetUserWidgetObject());
  
  FVector2D drawSize(933.f, 700.f);
  float desiredWorldHeight = 30.f;
  float scale = desiredWorldHeight / drawSize.Y;

  MainMenuWidgetComponent->SetDrawSize(drawSize);
  MainMenuWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));

  // spawn keyboard actor
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;
  Keyboard =
    GetWorld()->SpawnActor<AKeyboard>(
      AKeyboard::StaticClass(),
      GetActorLocation(),
      GetActorRotation(),
      spawnParams
    );
  Keyboard->AddActorWorldOffset(FVector(0.f, 0.f, (-desiredWorldHeight * 0.5f) - 1.f));
  Keyboard->AddActorLocalRotation(FRotator(10.f, 0.f, 0.f));
  Keyboard->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
  Keyboard->SetActorHiddenInGame(true);
  MainMenuWidget->SetKeyboard(Keyboard);
}

void AMainMenu::Init(
  TFunction<void ()> ExitFunction,
  TFunction<void ()> SaveFunction,
  TFunction<void (FString)> SaveAsFunction,
  TFunction<void ()> NewFunction,
  TFunction<void ()> ContinueFunction,
  TFunction<void ()> OverwriteTemplateFunction,
  TFunction<void (FString)> LoadFunction,
  TFunction<void (float)> CableOpacityUpdateFunction,
  TFunction<void (float)> CableTensionUpdateFunction,
  TFunction<void (bool)> CableColorCycleToggleFunction,
  TFunction<void (FString)> LoadMapFunction,
  TFunction<void (float)> EnvironmentLightIntensityUpdateFunction,
  TFunction<void (float)> EnvironmentLightAngleUpdateFunction
) {
  MainMenuWidget->SetExitFunction(ExitFunction);
  MainMenuWidget->SetSaveFunction(SaveFunction);
  MainMenuWidget->SetSaveAsFunction(SaveAsFunction);
  MainMenuWidget->SetNewFunction(NewFunction);
  MainMenuWidget->SetContinueFunction(ContinueFunction);
  MainMenuWidget->SetOverwriteTemplateFunction(OverwriteTemplateFunction);
  MainMenuWidget->SetLoadFunction(LoadFunction);
  MainMenuWidget->SetCableOpacityUpdateFunction(CableOpacityUpdateFunction);
  MainMenuWidget->SetCableTensionUpdateFunction(CableTensionUpdateFunction);
  MainMenuWidget->SetCableColorCycleToggleFunction(CableColorCycleToggleFunction);
  MainMenuWidget->SetLoadMapFunction(LoadMapFunction);
  MainMenuWidget->SetEnvironmentLightIntensityUpdateFunction(EnvironmentLightIntensityUpdateFunction);
  MainMenuWidget->SetEnvironmentLightAngleUpdateFunction(EnvironmentLightAngleUpdateFunction);

  // these don't change, so they don't need to be in Refresh,
  // but they do need to be set before we Refresh
  MainMenuWidget->SetFMShortcutsListItems(GenerateFMShortcutsEntries());

  URackSimGameUserSettings* UserSettings =
    Cast<URackSimGameUserSettings>(UGameUserSettings::GetGameUserSettings());
  if (UserSettings) MainMenuWidget->UpdateSettings(UserSettings);

  Refresh();
}

void AMainMenu::Hide() {
  MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
  SetActorHiddenInGame(true);
}

void AMainMenu::Show() {
  Refresh();
  MainMenuWidget->GotoMain();
  MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
  SetActorHiddenInGame(false);
}

void AMainMenu::Refresh() {
  MainMenuWidget->UpdateState(GameState);
  MainMenuWidget->SetRecentPatchesListItems(GenerateRecentPatchesEntries());
  MainMenuWidget->SetFMDrivesListItems(GenerateFMDrivesEntries());
}

void AMainMenu::Toggle() {
  if (IsHidden()) Show();
  else Hide();
}

void AMainMenu::Status(FString UpperText, FString LowerText) {
  MainMenuWidget->GotoStatus(UpperText, LowerText);
}

void AMainMenu::Confirm(
  FString ConfirmationLabel,
  FString ConfirmButtonLabel,
  TFunction<void ()> inConfirmationConfirmFunction
) {
  Show();
  MainMenuWidget->Confirm(
    ConfirmationLabel,
    ConfirmButtonLabel,
    inConfirmationConfirmFunction,
    [&]() {
      MainMenuWidget->GotoMain();
    },
    true // solo
  );
}

void AMainMenu::Alert(FString AlertLabel, FString ConfirmButtonLabel) {
  Show();
  MainMenuWidget->Alert(AlertLabel, ConfirmButtonLabel);
}

void AMainMenu::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (!IsHidden() && PlayerPawn) {
    SetActorRotation(PlayerPawn->GetLookAtCameraRotation(GetActorLocation()));

    SetActorLocation(GetActorLocation() + GetActorUpVector() * 10.f);
    SetActorLocation(
      FMath::VInterpTo(
        GetActorLocation(),
        PlayerPawn->GetMainMenuPosition(),
        DeltaTime,
        10.f
      )
    );
    SetActorLocation(GetActorLocation() + GetActorUpVector() * -10.f);
  }
}

TArray<UFileListEntryData*> AMainMenu::GenerateRecentPatchesEntries() {
  TArray<UFileListEntryData*> entries;

  for (FString path : GameMode->GetRecentPatchPaths()) {
    UFileListEntryData* entry = NewObject<UFileListEntryData>(this);
    entry->Label = FPaths::GetBaseFilename(path, true);
    entry->Path = path;
    entry->bScrollHover = true;
    entries.Add(entry);
  }

  return entries;
}

TArray<UFileListEntryData*> AMainMenu::GenerateFMDrivesEntries() {
  TArray<UFileListEntryData*> entries;

  TArray<TCHAR> driveLetters{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
  FString drivePath;
  for (TCHAR letter : driveLetters) {
    drivePath.Empty();
    drivePath.AppendChar(letter).Append(TEXT(":\\"));

    if (FPaths::DirectoryExists(drivePath)) {
      UFileListEntryData* entry = NewObject<UFileListEntryData>(this);
      entry->Label = drivePath;
      entry->Path = drivePath;
      entries.Add(entry);
    }
  }

  return entries;
}

TArray<UFileListEntryData*> AMainMenu::GenerateFMShortcutsEntries() {
  TArray<UFileListEntryData*> entries;

  FString homePath{"C:/Users/"};
  homePath.Append(FPlatformProcess::UserName()).Append(TEXT("/"));

  MainMenuWidget->LoadDirectoryInFileManager(homePath);

  FString desktopPath{homePath};
  desktopPath.Append(TEXT("Desktop/"));
  FString downloadsPath{homePath};
  downloadsPath.Append(TEXT("Downloads/"));

  FString documentsPath{FPlatformProcess::UserDir()};
  FString userSettingsPath{FPlatformProcess::UserSettingsDir()};

  UFileListEntryData* homeEntry = NewObject<UFileListEntryData>(this);
  homeEntry->Label = TEXT("Home");
  homeEntry->Path = homePath;
  entries.Add(homeEntry);

  FString patchesPath{userSettingsPath + "Rack2/patches/"};
  // fallback for rack < 2.5
  if (!FPaths::DirectoryExists(patchesPath))
    patchesPath = documentsPath + "Rack2/patches/";

  UFileListEntryData* patchesEntry = NewObject<UFileListEntryData>(this);
  patchesEntry->Label = TEXT("Rack2\\Patches");
  patchesEntry->Path = patchesPath;
  entries.Add(patchesEntry);

  UFileListEntryData* desktopEntry = NewObject<UFileListEntryData>(this);
  desktopEntry->Label = TEXT("Desktop");
  desktopEntry->Path = desktopPath;
  entries.Add(desktopEntry);

  UFileListEntryData* documentsEntry = NewObject<UFileListEntryData>(this);
  documentsEntry->Label = TEXT("Documents");
  documentsEntry->Path = documentsPath;
  entries.Add(documentsEntry);

  UFileListEntryData* downloadsEntry = NewObject<UFileListEntryData>(this);
  downloadsEntry->Label = TEXT("Downloads");
  downloadsEntry->Path = downloadsPath;
  entries.Add(downloadsEntry);

  return entries;
}

TArray<UFileListEntryData*> AMainMenu::GenerateFMFileBrowserEntries(FString& Directory) {
  TArray<UFileListEntryData*> entries;
  return entries;
}