#pragma once

#include "VCV.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVModule.generated.h"

class UTexture2D;
class Aosc3GameModeBase;
class AVCVLight;
class AVCVPort;

UCLASS()
class OSC3_API AVCVModule : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVModule();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void init(VCVModule model);
  
  void GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector);
  void AttachCable(const PortIdentity& identity, int64_t cableId);
  void DetachCable(const PortIdentity& identity, int64_t cableId);
  void UpdateLight(int32 lightId, FLinearColor color);

  UPROPERTY()
  TMap<int32, AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, AVCVLight*> ParamLightActors;
  
  void registerParamLight(int64_t lightId, AVCVLight* lightActor);
  void paramUpdated(int32 paramId, float value);

  FString getBrand();
  
private:
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY()
  UStaticMesh* StaticMesh;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  UPROPERTY()
  UTexture2D* texture;

  void spawnComponents();

  VCVModule model;
  
  Aosc3GameModeBase* gameMode;

  // TMap<int, class AVCVParam*> ParamActors;
  TMap<int, AVCVPort*> InputActors;
  TMap<int, AVCVPort*> OutputActors;
};