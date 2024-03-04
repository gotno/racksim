#include "osc3GameModeBase.h"
#include "osc3GameState.h"
#include "osc3PlayerController.h"
#include "OSCController.h"
#include "RackManager.h"
#include "MainMenu.h"
#include "Player/VRAvatar.h"
#include "osc3SaveGame.h"

#include "VCVModule.h"
#include "VCVCable.h"
#include "ModuleComponents/ContextMenu.h"
#include "ModuleComponents/VCVParam.h"
#include "ModuleComponents/VCVPort.h"
#include "Library.h"
#include "SVG/WidgetSurrogate.h"

#include "Engine/Texture2D.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "HeadMountedDisplayFunctionLibrary.h"

Aosc3GameModeBase::Aosc3GameModeBase() {
  OSCctrl = CreateDefaultSubobject<AOSCController>(FName(TEXT("OSCctrl")));
  rackman = CreateDefaultSubobject<URackManager>(FName(TEXT("rackman")));
}

void Aosc3GameModeBase::BeginPlay() {
	Super::BeginPlay();

  UHeadMountedDisplayFunctionLibrary::EnableHMD(true);

  GameState = Cast<Aosc3GameState>(UGameplayStatics::GetGameState(this));
  if (GameState) UE_LOG(LogTemp, Warning, TEXT("GameState exists"));
  
  rackman->Init();
  GameState->SetCanContinueAutosave(rackman->DoesAutosaveExist());

  PlayerController = Cast<Aosc3PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
  if (PlayerController) UE_LOG(LogTemp, Warning, TEXT("PlayerController exists"));

  PlayerPawn = Cast<AVRAvatar>(UGameplayStatics::GetPlayerPawn(this, 0));
  if (PlayerPawn) {
    UE_LOG(LogTemp, Warning, TEXT("PlayerPawn exists"));
    DefaultInPatchPlayerLocation =
      PlayerPawn->GetActorLocation() + FVector(0.f, 0.f, 606.f);
  }
  
  SpawnLibrary();
  SpawnMainMenu();
}

// new
//   RackIsRunning?
//   else
//     load rack with bootstrap
// continue (TODO: only if autosave dir exists)
//   load rack without bootstrap

void Aosc3GameModeBase::NewPatch() {
  SaveData = nullptr;

  if (rackman->RackIsRunning()) {
    // reset GameState
    // clear Modules/Cables
    // tell rack to load bootstrap
  } else {
    StartRack(true);
  }
}

void Aosc3GameModeBase::ContinueAutosave() {
  if (!UGameplayStatics::DoesSaveGameExist(TEXT("autosave"), 0)) {
    StartRack(false);
    return;
  }

  FAsyncLoadGameFromSlotDelegate LoadedDelegate;
  LoadedDelegate.BindLambda([&](const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData) {
    SaveData = Cast<Uosc3SaveGame>(LoadedGameData);
    UE_LOG(LogTemp, Warning, TEXT("player position: %s"), *SaveData->PlayerLocation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("library position: %s"), *SaveData->LibraryPosition.Location.ToString());
    PlayerPawn->SetActorLocation(SaveData->PlayerLocation);
    LibraryActor->SetActorLocation(SaveData->LibraryPosition.Location);
    LibraryActor->SetActorRotation(SaveData->LibraryPosition.Rotation);
    StartRack(false);
  });
  UGameplayStatics::AsyncLoadGameFromSlot(TEXT("autosave"), 0, LoadedDelegate);
}

void Aosc3GameModeBase::StartRack(bool bNewPatch) {
  rackman->Run(bNewPatch, [&]() {
    OSCctrl->Init();

    GameState->SetPatchLoaded(true);
    GameState->SetIsAutosave(!bNewPatch);

    if (!SaveData) PlayerPawn->SetActorLocation(DefaultInPatchPlayerLocation);
    PlayerPawn->EnableWorldManipulation();

    MainMenu->Hide();
  });
}

// TODO break into save/save+exit/exit
void Aosc3GameModeBase::RequestExit() {
  Uosc3SaveGame* SaveGame = MakeSaveGame();

  FAsyncSaveGameToSlotDelegate SavedDelegate;

  SavedDelegate.BindLambda([&](const FString& SlotName, const int32 UserIndex, bool bSuccess) {
    float exitDelay{0.1f}; // seconds

    if (OSCctrl->IsRunning() && rackman->RackIsRunning()) {
      // give rack a second to finish exiting before EndPlay runs Cleanup
      exitDelay = 1.f;

      OSCctrl->SendAutosaveAndExit();
    }

    FTimerHandle hExit;
    GetWorld()->GetTimerManager().SetTimer(
      hExit,
      this,
      &Aosc3GameModeBase::Exit,
      exitDelay,
      false // loop
    );
  });

  // TODO GameState->patch path or something
  UGameplayStatics::AsyncSaveGameToSlot(SaveGame, TEXT("autosave"), 0, SavedDelegate);
}

void Aosc3GameModeBase::ToggleMainMenu() {
  MainMenu->Toggle();
}

void Aosc3GameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  rackman->Cleanup();
}

Uosc3SaveGame* Aosc3GameModeBase::MakeSaveGame() {
  if (Uosc3SaveGame* SaveGameInstance = Cast<Uosc3SaveGame>(UGameplayStatics::CreateSaveGameObject(Uosc3SaveGame::StaticClass()))) {
    for (const TPair<int64, AVCVModule*>& pair : ModuleActors) {
      FVCVModuleInfo info;
      AVCVModule* module = pair.Value;
      module->GetModulePosition(info.Position.Location, info.Position.Rotation);
      SaveGameInstance->ModuleInfos.Add(module->Id, info);
    }

    LibraryActor->GetPosition(SaveGameInstance->LibraryPosition.Location, SaveGameInstance->LibraryPosition.Rotation);
    SaveGameInstance->PlayerLocation = PlayerPawn->GetActorLocation();

    return SaveGameInstance;
  }
  return nullptr;
}

void Aosc3GameModeBase::Exit() {
  UKismetSystemLibrary::QuitGame(
    GetWorld(),
    UGameplayStatics::GetPlayerController(this, 0),
    EQuitPreference::Quit,
    false
  );
}

void Aosc3GameModeBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void Aosc3GameModeBase::SpawnModule(VCVModule vcv_module) {
  if (ModuleActors.Contains(vcv_module.id)) return;

  FVector location;
  FRotator rotation;

  if (vcv_module.returnId == 0 && ModuleActors.Contains(LastClickedMenuModuleId)) {
    AVCVModule* lastClickedModule = ModuleActors[LastClickedMenuModuleId];
    lastClickedModule->GetModuleLandingPosition(location, rotation, true);
  } else if (vcv_module.returnId == -1 && SaveData && SaveData->ModuleInfos.Contains(vcv_module.id)) {
    location = SaveData->ModuleInfos[vcv_module.id].Position.Location;
    rotation = SaveData->ModuleInfos[vcv_module.id].Position.Rotation;
  } else if (ReturnModulePositions.Contains(vcv_module.returnId)) {
    ReturnModulePosition& rmp = ReturnModulePositions[vcv_module.returnId];
    if (rmp.Location.IsZero()) {
      LibraryActor->GetModuleLandingPosition(vcv_module.box.size.x, location, rotation);
    } else {
      location = rmp.Location;
      rotation = rmp.Rotation;
    }
    ReturnModulePositions.Remove(vcv_module.returnId);
  } else { 
    location = FVector(0, vcv_module.box.pos.x, -vcv_module.box.pos.y + 140);
    location.Y += vcv_module.box.size.x / 2;
    location.Z += vcv_module.box.size.y / 2;
    rotation = FRotator(0.f);
  }

  AVCVModule* module =
    GetWorld()->SpawnActor<AVCVModule>(
      AVCVModule::StaticClass(),
      location,
      FRotator(0.f)
    );
 
  ModuleActors.Add(vcv_module.id, module);
  module->Init(vcv_module);

  module->SetActorRotation(rotation);

  ProcessSpawnCableQueue();
}

void Aosc3GameModeBase::QueueCableSpawn(VCVCable vcv_cable) {
  cableQueue.Push(vcv_cable);
  ProcessSpawnCableQueue();
}

void Aosc3GameModeBase::ProcessSpawnCableQueue() {
  TArray<VCVCable> spawnedCables;
  
  bool anyUnpersistedCables = CableActors.ContainsByPredicate([](AVCVCable* Cable) {
    return Cable->Id == -1;
  });

  for (VCVCable cable : cableQueue) {
    AVCVCable* matchingUnpersistedCable{nullptr};
    
    if (anyUnpersistedCables) {
      for (AVCVCable* cableToCheck : CableActors) {
        if (cableToCheck->Id != -1) continue;

        AVCVPort* inputPort = cableToCheck->GetPort(PortType::Input);
        if (!inputPort) continue;
        
        if (inputPort->Id == cable.inputPortId && inputPort->Module->Id == cable.inputModuleId) {
          matchingUnpersistedCable = cableToCheck;
          break;
        }
      }
    }

    if (matchingUnpersistedCable) {
      matchingUnpersistedCable->SetId(cable.id);
      spawnedCables.Push(cable);
    } else if (ModuleActors.Contains(cable.inputModuleId) && ModuleActors.Contains(cable.outputModuleId)) {
      SpawnCable(
        cable.id,
        ModuleActors[cable.inputModuleId]->GetPortActor(PortType::Input, cable.inputPortId),
        ModuleActors[cable.outputModuleId]->GetPortActor(PortType::Output, cable.outputPortId)
      );
      spawnedCables.Push(cable);
    } else {
      // UE_LOG(LogTemp, Warning, TEXT("awaiting modules %lld:%lld for cable %lld"), cable.inputModuleId, cable.outputModuleId, cable.id);
    }
  }

  for (VCVCable cable : spawnedCables) {
    cableQueue.RemoveSwap(cable);
  }
}

// spawn unpersisted cable attached to port
AVCVCable* Aosc3GameModeBase::SpawnCable(AVCVPort* Port) {
  AVCVCable* cable =
    GetWorld()->SpawnActor<AVCVCable>(
      AVCVCable::StaticClass(),
      FVector(0, 0, 0),
      FRotator(0, 0, 0)
    );
  cable->SetPort(Port);
  
  CableActors.Push(cable);
  return cable;
}

// spawn complete/persisted cable
void Aosc3GameModeBase::SpawnCable(int64_t& Id, AVCVPort* InputPort, AVCVPort* OutputPort) {
  AVCVCable* cable = SpawnCable(InputPort);
  cable->SetId(Id);
  cable->SetPort(OutputPort);
}

void Aosc3GameModeBase::RegisterCableConnect(AVCVPort* InputPort, AVCVPort* OutputPort) {
  OSCctrl->CreateCable(InputPort->Module->Id, OutputPort->Module->Id, InputPort->Id, OutputPort->Id);
}

void Aosc3GameModeBase::RegisterCableDisconnect(AVCVCable* Cable) {
  OSCctrl->DestroyCable(Cable->Id);
  Cable->SetId(-1);
}

void Aosc3GameModeBase::DestroyCableActor(AVCVCable* Cable) {
  CableActors.Remove(Cable);
  Cable->Destroy();
}

void Aosc3GameModeBase::DuplicateModule(AVCVModule* Module) {
  int returnId = ++currentReturnModuleId;

  FVector location;
  FRotator rotation;
  Module->GetModuleLandingPosition(location, rotation, false);
  ReturnModulePositions.Add(returnId, ReturnModulePosition(location, rotation));

  FString pluginSlug, moduleSlug;
  Module->GetSlugs(pluginSlug, moduleSlug);

  OSCctrl->SendCreateModule(pluginSlug, moduleSlug, returnId);
}

void Aosc3GameModeBase::RequestModuleSpawn(FString PluginSlug, FString ModuleSlug) {
  int returnId = ++currentReturnModuleId;

  ReturnModulePositions.Add(returnId, ReturnModulePosition());
  OSCctrl->SendCreateModule(PluginSlug, ModuleSlug, returnId);
}

void Aosc3GameModeBase::RequestModuleDiff(const int64_t& ModuleId) const {
  OSCctrl->SendModuleDiffRequest(ModuleId);
};

void Aosc3GameModeBase::SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite) {
  OSCctrl->SetModuleFavorite(PluginSlug, ModuleSlug, bFavorite);
  LibraryActor->SetModuleFavorite(PluginSlug, ModuleSlug, bFavorite);
}

void Aosc3GameModeBase::DestroyModule(int64_t ModuleId, bool bSync) {
  if (!ModuleActors.Contains(ModuleId)) return;

  ModuleActors[ModuleId]->Destroy();
  ModuleActors.Remove(ModuleId);

  if (bSync) OSCctrl->SendDestroyModule(ModuleId);
}

void Aosc3GameModeBase::RequestMenu(const FVCVMenu& Menu) const {
  OSCctrl->RequestMenu(Menu);
}

void Aosc3GameModeBase::ClickMenuItem(const FVCVMenuItem& MenuItem) {
  LastClickedMenuModuleId = MenuItem.moduleId;
  OSCctrl->ClickMenuItem(MenuItem);
}

void Aosc3GameModeBase::UpdateMenuItemQuantity(const FVCVMenuItem& MenuItem, const float& Value) const {
  OSCctrl->UpdateMenuItemQuantity(MenuItem, Value);
}

void Aosc3GameModeBase::UpdateLight(int64_t ModuleId, int32 LightId, FLinearColor Color) {
  if (!ModuleActors.Contains(ModuleId)) return;
  ModuleActors[ModuleId]->UpdateLight(LightId, Color);
}

void Aosc3GameModeBase::UpdateParam(int64_t ModuleId, VCVParam& Param) {
  if (!ModuleActors.Contains(ModuleId)) return;
  ModuleActors[ModuleId]->GetParamActor(Param.id)->Update(Param);
}

void Aosc3GameModeBase::SendParamUpdate(int64_t ModuleId, int32 ParamId, float Value) {
  OSCctrl->SendParamUpdate(ModuleId, ParamId, Value);
}

void Aosc3GameModeBase::RegisterSVG(FString Filepath, Vec2 Size) {
  if (Filepath.Compare(FString("")) == 0) return;
  if (SVGAssets.Contains(Filepath)) return;

  UE_LOG(LogTemp, Warning, TEXT("importing svg %s"), *Filepath);

  UDPSVGAsset* svgAsset = NewObject<UDPSVGAsset>(this, UDPSVGAsset::StaticClass());
  SVGImporter.PerformImport(Filepath, svgAsset);
  SVGAssets.Add(Filepath, svgAsset);
  
  FVector surrogateLocation;
  FRotator surrogateRotation;
  PlayerPawn->GetRenderablePosition(surrogateLocation, surrogateRotation);

  AWidgetSurrogate* surrogate = 
    GetWorld()->SpawnActor<AWidgetSurrogate>(
      AWidgetSurrogate::StaticClass(),
      surrogateLocation,
      surrogateRotation
    );
  
  SVGWidgetSurrogates.Add(Filepath, surrogate);
  surrogate->SetSVG(svgAsset, Size, Filepath);
  surrogate->SetActorScale3D(FVector(0.05f, 0.05f, 0.05f));
}

void Aosc3GameModeBase::RegisterTexture(FString Filepath, UTexture2D* Texture) {
  UE_LOG(LogTemp, Warning, TEXT("registering texture %s"), *Filepath);
  
  SVGTextures.Add(Filepath, Texture);
  // use this to short-circuit surrogate destruction to preview rendering of a given svg
  // if (Filepath.Compare(FString("C:/VCV/rack-src/rack-gotno/res/ComponentLibrary/VCVSlider.svg")) == 0) return;
  SVGWidgetSurrogates[Filepath]->Destroy();
  SVGWidgetSurrogates.Remove(Filepath);
}

UTexture2D* Aosc3GameModeBase::GetTexture(FString Filepath) {
  if (!SVGTextures.Contains(Filepath)) return nullptr;
  return SVGTextures[Filepath];
}

void Aosc3GameModeBase::SpawnLibrary() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  LibraryActor =
    GetWorld()->SpawnActor<ALibrary>(
      ALibrary::StaticClass(),
      FVector(100.f, 100.f, 0.f),
      FRotator(0.f),
      spawnParams
    );
  // LibraryActor->SetActorHiddenInGame(true);
}

void Aosc3GameModeBase::SpawnMainMenu() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  MainMenu =
    GetWorld()->SpawnActor<AMainMenu>(
      AMainMenu::StaticClass(),
      FVector(0.f, 0.f, -600.f),
      FRotator(0.f),
      spawnParams
    );
  MainMenu->Init(
    PlayerPawn,
    GameState,
    [&]() { RequestExit(); }, // 'exit' button callback
    [&]() { NewPatch(); }, // 'new patch' button callback
    [&]() { ContinueAutosave(); } // 'continue with autosave' button callback
  );
}

ALibrary* Aosc3GameModeBase::GetLibrary() {
  if (LibraryActor) return LibraryActor;
  return nullptr;
}

void Aosc3GameModeBase::SetLibraryJsonPath(FString& Path) {
  LibraryActor->SetJsonPath(Path);
}

void Aosc3GameModeBase::SubscribeMenuItemSyncedDelegate(AContextMenu* ContextMenu) {
  OSCctrl->OnMenuItemSyncedDelegate.AddUObject(ContextMenu, &AContextMenu::AddMenuItem);
}

void Aosc3GameModeBase::SubscribeMenuSyncedDelegate(AContextMenu* ContextMenu) {
  OSCctrl->OnMenuSyncedDelegate.AddUObject(ContextMenu, &AContextMenu::MenuSynced);
}