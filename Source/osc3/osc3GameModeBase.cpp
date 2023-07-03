#include "osc3GameModeBase.h"
#include "osc3PlayerController.h"
#include "Avatar.h"

#include "DrawDebugHelpers.h"

#include "VCV.h"
#include "VCVModule.h"
#include "VCVCable.h"
#include "VCVLight.h"
#include "VCVParam.h"

#include "IndicatorHUD.h"

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

void Aosc3GameModeBase::SpawnCable(VCVCable cable) {
  AVCVCable* a_cable =
    GetWorld()->SpawnActor<AVCVCable>(
      AVCVCable::StaticClass(),
      FVector(0, 0, 0),
      FRotator(0, 0, 0)
    );
  
  CableActors.Add(cable.id, a_cable);
  a_cable->init(cable);
  
  const PortIdentity& inputIdentity = cable.getInputIdentity();
  const PortIdentity& outputIdentity = cable.getOutputIdentity();
  ModuleActors[inputIdentity.moduleId]->ConnectCable(inputIdentity, cable.id);
  ModuleActors[outputIdentity.moduleId]->ConnectCable(outputIdentity, cable.id);
}

void Aosc3GameModeBase::QueueCableSpawn(VCVCable cable) {
  cableQueue.Push(cable);
}

void Aosc3GameModeBase::ProcessSpawnCableQueue() {
  TArray<VCVCable> spawnedCables;

  for (VCVCable cable : cableQueue) {
    int64_t inputModuleId = cable.getInputIdentity().moduleId;
    int64_t outputModuleId = cable.getOutputIdentity().moduleId;

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

void Aosc3GameModeBase::UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color) {
  if (ModuleActors.Contains(moduleId)) {
    ModuleActors[moduleId]->UpdateLight(lightId, color);
  }
}

bool Aosc3GameModeBase::GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector) {
  if (!ModuleActors.Contains(identity.moduleId)) return false;

  AVCVModule* module = ModuleActors[identity.moduleId];
  module->GetPortInfo(identity, portLocation, portForwardVector);
  return true;
}

void Aosc3GameModeBase::SendParamUpdate(int64_t moduleId, int32 paramId, float value) {
  OSCctrl->SendParamUpdate(moduleId, paramId, value);
}