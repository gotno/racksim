#include "osc3GameModeBase.h"
#include "osc3PlayerController.h"
#include "Avatar.h"
#include "VRAvatar.h"

#include "VCV.h"
#include "VCVOverrides.h"
#include "VCVModule.h"
#include "VCVCable.h"
#include "VCVParam.h"
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
  OSCctrl->NotifyResync();

  PlayerController = Cast<Aosc3PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
  PlayerPawn = Cast<AVRAvatar>(UGameplayStatics::GetPlayerPawn(this, 0));
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

AVCVCable* Aosc3GameModeBase::SpawnCable(VCVCable cable) {
  AVCVCable* a_cable =
    GetWorld()->SpawnActor<AVCVCable>(
      AVCVCable::StaticClass(),
      FVector(0, 0, 0),
      FRotator(0, 0, 0)
    );
  
  CableActors.Add(cable.id, a_cable);
  a_cable->init(cable);
  
  PortIdentity inputIdentity = cable.getIdentity(PortType::Input);
  if (!inputIdentity.isNull() && ModuleActors.Contains(inputIdentity.moduleId))
    ModuleActors[inputIdentity.moduleId]->AttachCable(inputIdentity, cable.id);

  PortIdentity outputIdentity = cable.getIdentity(PortType::Output);
  if (!outputIdentity.isNull() && ModuleActors.Contains(outputIdentity.moduleId))
    ModuleActors[outputIdentity.moduleId]->AttachCable(outputIdentity, cable.id);

  return a_cable;
}

void Aosc3GameModeBase::DestroyCable(int64_t cableId) {
  if (!CableActors.Contains(cableId)) return;

  VCVCable cable = CableActors[cableId]->getModel();

  PortIdentity inputIdentity = cable.getIdentity(PortType::Input);
  if (!inputIdentity.isNull())
    ModuleActors[inputIdentity.moduleId]->DetachCable(inputIdentity, cable.id);

  PortIdentity outputIdentity = cable.getIdentity(PortType::Output);
  if (!outputIdentity.isNull())
    ModuleActors[outputIdentity.moduleId]->DetachCable(outputIdentity, cable.id);

  CableActors[cableId]->Destroy();
  CableActors.Remove(cableId);
}

void Aosc3GameModeBase::QueueCableSpawn(VCVCable cable) {
  cableQueue.Push(cable);
  ProcessSpawnCableQueue();
}

void Aosc3GameModeBase::ProcessSpawnCableQueue() {
  TArray<VCVCable> spawnedCables;

  for (VCVCable cable : cableQueue) {
    int64_t inputModuleId = cable.getIdentity(PortType::Input).moduleId;
    int64_t outputModuleId = cable.getIdentity(PortType::Output).moduleId;

    if (ModuleActors.Contains(inputModuleId) && ModuleActors.Contains(outputModuleId)) {
      SpawnCable(cable);
      spawnedCables.Push(cable);
    } else {
      UE_LOG(LogTemp, Warning, TEXT("awaiting modules %lld:%lld for cable %lld"), inputModuleId, outputModuleId, cable.id);
    }
  }

  for (VCVCable cable : spawnedCables) {
    cableQueue.RemoveSwap(cable);
  }
  spawnedCables.Empty();
}

AVCVCable* Aosc3GameModeBase::DetachCable(int64_t cableId, PortIdentity identity) {
  OSCctrl->DestroyCable(cableId);
  AVCVCable* cable = CableActors[cableId];
  cable->disconnectFrom(identity);
  
  return cable;
}

void Aosc3GameModeBase::AttachCable(int64_t cableId, PortIdentity identity) {
  CableActors[cableId]->connectTo(identity);

  VCVCable cable = CableActors[cableId]->getModel();

  PortIdentity inputIdentity = cable.getIdentity(PortType::Input);
  PortIdentity outputIdentity = cable.getIdentity(PortType::Output);

  OSCctrl->CreateCable(
    inputIdentity.moduleId,
    outputIdentity.moduleId,
    inputIdentity.portId,
    outputIdentity.portId
  );

  DestroyCable(cable.id);
}

void Aosc3GameModeBase::DuplicateModule(AVCVModule* Module) {
  FString pluginSlug, moduleSlug;
  Module->GetSlugs(pluginSlug, moduleSlug);
  OSCctrl->CreateModule(pluginSlug, moduleSlug);
}

void Aosc3GameModeBase::RequestModuleSpawn(FString PluginSlug, FString ModuleSlug) {
  OSCctrl->CreateModule(PluginSlug, ModuleSlug);
}

void Aosc3GameModeBase::DestroyModule(AVCVModule* Module) {
  ModuleActors[Module->GetId()]->Destroy();
  ModuleActors.Remove(Module->GetId());
  OSCctrl->DestroyModule(Module->GetId());
}

void Aosc3GameModeBase::UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color) {
  if (ModuleActors.Contains(moduleId)) {
    ModuleActors[moduleId]->UpdateLight(lightId, color);
  }
}

void Aosc3GameModeBase::UpdateParamDisplayValue(int64_t moduleId, int32 paramId, FString displayValue) {
  if (ModuleActors.Contains(moduleId)) {
    ModuleActors[moduleId]->GetParamActor(paramId)->UpdateDisplayValue(displayValue);
  }
}

void Aosc3GameModeBase::GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector) {
  AVCVModule* module = ModuleActors[identity.moduleId];
  module->GetPortInfo(identity, portLocation, portForwardVector);
}

AVCVPort* Aosc3GameModeBase::GetPortActor(PortIdentity identity) {
  AVCVModule* module = ModuleActors[identity.moduleId];
  return module->GetPortActor(identity);
}

void Aosc3GameModeBase::SendParamUpdate(int64_t moduleId, int32 paramId, float value) {
  OSCctrl->SendParamUpdate(moduleId, paramId, value);
}

void Aosc3GameModeBase::RegisterSVG(FString filepath, Vec2 size) {
  if (filepath.Compare(FString("")) == 0) return;
  if (SVGAssets.Contains(filepath)) return;

  // UE_LOG(LogTemp, Warning, TEXT("importing svg %s"), *filepath);

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

void Aosc3GameModeBase::SpawnLibrary(VCVLibrary& vcv_library) {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  LibraryActor = GetWorld()->SpawnActor<ALibrary>(
      ALibrary::StaticClass(),
      FVector(0, 0, 100.f),
      FRotator(0, 180.f, 0),
      spawnParams
    );
  LibraryActor->Init(vcv_library);
}