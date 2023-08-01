#include "osc3GameModeBase.h"
#include "osc3PlayerController.h"
#include "Avatar.h"

#include "VCV.h"
#include "VCVModule.h"
#include "VCVCable.h"
#include "WidgetSurrogate.h"

// #include "IndicatorHUD.h"

#include "Engine/Texture2D.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"
#include "Kismet/GameplayStatics.h"

Aosc3GameModeBase::Aosc3GameModeBase() {
  PlayerControllerClass = Aosc3PlayerController::StaticClass();
  DefaultPawnClass = AAvatar::StaticClass();

  OSCctrl = CreateDefaultSubobject<AOSCController>(FName(TEXT("OSCctrl")));
  // HUDClass = AIndicatorHUD::StaticClass();
}

void Aosc3GameModeBase::BeginPlay() {
	Super::BeginPlay();

  OSCctrl->Init();
  OSCctrl->NotifyResync();

  PlayerController = Cast<Aosc3PlayerController>(UGameplayStatics::GetPlayerController(this, 0));
  PlayerPawn = Cast<AAvatar>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void Aosc3GameModeBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
}

void Aosc3GameModeBase::SpawnModule(VCVModule module) {
  if (ModuleActors.Contains(module.id)) return;

  spawnXPositionCursor += 1 + module.box.size.x / 2;

  if (spawnXPositionCursor > 30) {
    spawnXPositionCursor = 0.f;
    spawnYPositionCursor += 1 + module.box.size.y;
  }

  AVCVModule* a_module =
    GetWorld()->SpawnActor<AVCVModule>(
      AVCVModule::StaticClass(),
      FVector(0, spawnXPositionCursor, spawnYPositionCursor),
      FRotator(0, 0, 0)
    );
  
  ModuleActors.Add(module.id, a_module);
  a_module->init(module);

  spawnXPositionCursor += module.box.size.x / 2;

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
  if (!inputIdentity.isNull())
    ModuleActors[inputIdentity.moduleId]->AttachCable(inputIdentity, cable.id);

  PortIdentity outputIdentity = cable.getIdentity(PortType::Output);
  if (!outputIdentity.isNull())
    ModuleActors[outputIdentity.moduleId]->AttachCable(outputIdentity, cable.id);

  return a_cable;
}

void Aosc3GameModeBase::DestroyCable(int64_t cableId) {
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
  // ModuleActors[identity.moduleId]->AttachCable(identity, cableId);

  VCVCable cable = CableActors[cableId]->getModel();

  PortIdentity inputIdentity = cable.getIdentity(PortType::Input);
  PortIdentity outputIdentity = cable.getIdentity(PortType::Output);

  OSCctrl->CreateCable(
    inputIdentity.moduleId,
    outputIdentity.moduleId,
    inputIdentity.portId,
    outputIdentity.portId
  );
  // destroy cable actor & send create cable
  // 
  // but in the meantime,
  // CableActors[cableId]->connectTo(identity);
  DestroyCable(cable.id);
}

void Aosc3GameModeBase::UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color) {
  if (ModuleActors.Contains(moduleId)) {
    ModuleActors[moduleId]->UpdateLight(lightId, color);
  }
}

void Aosc3GameModeBase::GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector) {
  // if (!ModuleActors.Contains(identity.moduleId)) return false;

  AVCVModule* module = ModuleActors[identity.moduleId];
  module->GetPortInfo(identity, portLocation, portForwardVector);
  // return true;
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