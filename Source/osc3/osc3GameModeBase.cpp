#include "osc3GameModeBase.h"
#include "osc3PlayerController.h"
#include "OSCController.h"
#include "VRAvatar.h"

#include "VCVModule.h"
#include "VCVCable.h"
#include "ModuleComponents/VCVParam.h"
#include "ModuleComponents/VCVPort.h"
#include "Library.h"
#include "WidgetSurrogate.h"

#include "Engine/Texture2D.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"
#include "Kismet/GameplayStatics.h"

Aosc3GameModeBase::Aosc3GameModeBase() {
  PlayerControllerClass = Aosc3PlayerController::StaticClass();

  OSCctrl = CreateDefaultSubobject<AOSCController>(FName(TEXT("OSCctrl")));
}

void Aosc3GameModeBase::BeginPlay() {
	Super::BeginPlay();

  OSCctrl->Init();

  FTimerHandle resyncHandle;
  GetWorld()->GetTimerManager().SetTimer(
    resyncHandle,
    OSCctrl,
    &AOSCController::NotifyResync,
    1.f, // seconds, apparently
    false
  );

  PlayerController = Cast<Aosc3PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
  PlayerPawn = Cast<AVRAvatar>(UGameplayStatics::GetPlayerPawn(this, 0));
  
  SpawnLibrary();
}

void Aosc3GameModeBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
}

void Aosc3GameModeBase::SpawnModule(VCVModule module) {
  if (ModuleActors.Contains(module.id)) return;

  FVector position = FVector(0, module.box.pos.x, -module.box.pos.y + 140);
  position.Y += module.box.size.x / 2;
  position.Z += module.box.size.y / 2;

  AVCVModule* a_module =
    GetWorld()->SpawnActor<AVCVModule>(
      AVCVModule::StaticClass(),
      position,
      FRotator(0, 0, 0)
    );
 
  ModuleActors.Add(module.id, a_module);
  a_module->init(module);

  ProcessSpawnCableQueue();
}

void Aosc3GameModeBase::QueueCableSpawn(VCVCable cable) {
  cableQueue.Push(cable);
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
  FString pluginSlug, moduleSlug;
  Module->GetSlugs(pluginSlug, moduleSlug);
  OSCctrl->CreateModule(pluginSlug, moduleSlug);
}

void Aosc3GameModeBase::RequestModuleSpawn(FString PluginSlug, FString ModuleSlug) {
  OSCctrl->CreateModule(PluginSlug, ModuleSlug);
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

void Aosc3GameModeBase::RequestMenu(const VCVMenu& Menu) const {
  OSCctrl->RequestMenu(Menu);
}

void Aosc3GameModeBase::ClickMenuItem(const VCVMenuItem& MenuItem) const {
  OSCctrl->ClickMenuItem(MenuItem);
}

void Aosc3GameModeBase::UpdateMenuItemQuantity(const VCVMenuItem& MenuItem, const float& Value) const {
  OSCctrl->UpdateMenuItemQuantity(MenuItem, Value);
}

void Aosc3GameModeBase::UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color) {
  if (!ModuleActors.Contains(moduleId)) return;
  ModuleActors[moduleId]->UpdateLight(lightId, color);
}

void Aosc3GameModeBase::UpdateParam(int64_t moduleId, VCVParam& param) {
  if (!ModuleActors.Contains(moduleId)) return;
  ModuleActors[moduleId]->GetParamActor(param.id)->Update(param);
}

void Aosc3GameModeBase::UpdateModuleMenuItem(VCVMenuItem& MenuItem) {
  if (!ModuleActors.Contains(MenuItem.moduleId)) return;
  ModuleActors[MenuItem.moduleId]->AddMenuItem(MenuItem);
}

void Aosc3GameModeBase::ModuleMenuSynced(VCVMenu& Menu) {
  if (!ModuleActors.Contains(Menu.moduleId)) return;
  ModuleActors[Menu.moduleId]->MenuSynced(Menu);
}

void Aosc3GameModeBase::SendParamUpdate(int64_t moduleId, int32 paramId, float value) {
  OSCctrl->SendParamUpdate(moduleId, paramId, value);
}

void Aosc3GameModeBase::RegisterSVG(FString filepath, Vec2 size) {
  if (filepath.Compare(FString("")) == 0) return;
  if (SVGAssets.Contains(filepath)) return;

  UE_LOG(LogTemp, Warning, TEXT("importing svg %s"), *filepath);

  UDPSVGAsset* svgAsset = NewObject<UDPSVGAsset>(this, UDPSVGAsset::StaticClass());
  SVGImporter.PerformImport(filepath, svgAsset);
  SVGAssets.Add(filepath, svgAsset);
  
  FVector surrogateLocation;
  FRotator surrogateRotation;
  PlayerPawn->GetRenderablePosition(surrogateLocation, surrogateRotation);

  AWidgetSurrogate* surrogate = 
    GetWorld()->SpawnActor<AWidgetSurrogate>(
      AWidgetSurrogate::StaticClass(),
      surrogateLocation,
      surrogateRotation
    );
  
  SVGWidgetSurrogates.Add(filepath, surrogate);
  surrogate->SetSVG(svgAsset, size, filepath);
  surrogate->SetActorScale3D(FVector(0.05f, 0.05f, 0.05f));
}

void Aosc3GameModeBase::RegisterTexture(FString filepath, UTexture2D* texture) {
  UE_LOG(LogTemp, Warning, TEXT("registering texture %s"), *filepath);
  
  SVGTextures.Add(filepath, texture);
  // use this to short-circuit surrogate destruction to preview rendering of a given svg
  // if (filepath.Compare(FString("C:/VCV/rack-src/rack-gotno/res/ComponentLibrary/VCVSlider.svg")) == 0) return;
  SVGWidgetSurrogates[filepath]->Destroy();
  SVGWidgetSurrogates.Remove(filepath);
}

UTexture2D* Aosc3GameModeBase::GetTexture(FString filepath) {
  if (!SVGTextures.Contains(filepath)) return nullptr;
  return SVGTextures[filepath];
}

void Aosc3GameModeBase::SpawnLibrary() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  LibraryActor =
    GetWorld()->SpawnActor<ALibrary>(
      ALibrary::StaticClass(),
      FVector(0, 0, 100.f),
      FRotator(0, 180.f, 0),
      spawnParams
    );
}

ALibrary* Aosc3GameModeBase::GetLibrary() {
  if (LibraryActor) return LibraryActor;
  return nullptr;
}

void Aosc3GameModeBase::SetLibraryJsonPath(FString& Path) {
  LibraryActor->SetJsonPath(Path);
}