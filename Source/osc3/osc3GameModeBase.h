#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "OSCController.h"
#include "unordered_map"

#include "osc3GameModeBase.generated.h"

UCLASS()
class OSC3_API Aosc3GameModeBase : public AGameMode {
	GENERATED_BODY()

public:
    Aosc3GameModeBase();

private:
  virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
  void SpawnModule(struct VCVModule module);
  void QueueCableSpawn(VCVCable cable);

  bool GetPortInfo(
    PortIdentity identity,
    FVector& portLocation,
    FVector& portForwardVector
  );

  void UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color);

  void SendParamUpdate(int64_t moduleId, int32 paramId, float value);
private:
  UPROPERTY()
  AOSCController* OSCctrl;

  class Aosc3PlayerController* PlayerController;
  
  float spawnXPositionCursor = 31.f;
  float spawnYPositionCursor = 0.f;

  void SpawnCable(struct VCVCable cable);
  void ProcessSpawnCableQueue();
  TArray<VCVCable> cableQueue;

  UPROPERTY()
  TMap<int64, class AVCVModule*> ModuleActors;
  UPROPERTY()
  TMap<int64, class AVCVCable*> CableActors;
  
};
