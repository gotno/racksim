#pragma once

#include "CoreMinimal.h"

#include "OSCController.h"
#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"

#include "GameFramework/GameMode.h"
#include "osc3GameModeBase.generated.h"

struct VCVModule;
struct VCVCable;
class AAvatar;
class AVRAvatar;
class AVCVCable;
class AVCVModule;
class AWidgetSurrogate;
class Aosc3PlayerController;
class UDPSVGAsset;
class UTexture2D;

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
  
  void RegisterSVG(FString filepath, Vec2 size);
  void RegisterTexture(FString filepath, UTexture2D* texture);
  UTexture2D* GetTexture(FString filepath);
private:
  UPROPERTY()
  AOSCController* OSCctrl;

  Aosc3PlayerController* PlayerController;
  // AAvatar* PlayerPawn;
  AVRAvatar* PlayerPawn;
  
  float spawnXPositionCursor{0.f};
  float spawnYPositionCursor{150.f};

  void ProcessSpawnCableQueue();
  TArray<VCVCable> cableQueue;

  UPROPERTY()
  TMap<int64, AVCVModule*> ModuleActors;
  UPROPERTY()
  TMap<int64, AVCVCable*> CableActors;

  FDPSVGImporter SVGImporter;
  UPROPERTY()
  TMap<FString, UDPSVGAsset*> SVGAssets;
  UPROPERTY()
  TMap<FString, AWidgetSurrogate*> SVGWidgetSurrogates;
  UPROPERTY()
  TMap<FString, UTexture2D*> SVGTextures;
};
