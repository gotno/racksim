#pragma once

#include "CoreMinimal.h"

#include "OSCController.h"
#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "GameFramework/GameMode.h"
#include "osc3GameModeBase.generated.h"

class AVCVCable;
class UDPSVGAsset;
class Aosc3PlayerController;
struct VCVModule;
struct VCVCable;

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
  void SpawnModule(VCVModule module);
  void QueueCableSpawn(VCVCable cable);

  void GetPortInfo(
    PortIdentity identity,
    FVector& portLocation,
    FVector& portForwardVector
  );
  AVCVCable* SpawnCable(VCVCable cable);
  void DestroyCable(int64_t cableId);
  AVCVCable* DetachCable(int64_t cableId, PortIdentity identity);
  void AttachCable(int64_t cableId, PortIdentity identity);

  void UpdateLight(int64_t moduleId, int32 lightId, FLinearColor color);

  void SendParamUpdate(int64_t moduleId, int32 paramId, float value);
  
  void RegisterSVG(FString filepath);
  UDPSVGAsset* GetSVGAsset(FString filepath);
private:
  UPROPERTY()
  AOSCController* OSCctrl;

  Aosc3PlayerController* PlayerController;
  
  float spawnXPositionCursor = 31.f;
  float spawnYPositionCursor = 0.f;

  void ProcessSpawnCableQueue();
  TArray<VCVCable> cableQueue;

  UPROPERTY()
  TMap<int64, class AVCVModule*> ModuleActors;
  UPROPERTY()
  TMap<int64, AVCVCable*> CableActors;

  FDPSVGImporter SVGImporter;
  UPROPERTY()
  TMap<FString, UDPSVGAsset*> SVGAssets;
};
