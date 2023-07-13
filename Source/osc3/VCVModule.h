#pragma once

#include "VCV.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVModule.generated.h"

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
  TMap<int32, class AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, class AVCVLight*> ParamLightActors;
  
  void registerParamLight(int64_t lightId, AVCVLight* lightActor);
  void paramUpdated(int32 paramId, float value);
  
private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  USceneComponent* SceneComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* StaticMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* MaterialInstance;

  UPROPERTY()
  UMaterialInterface* MaterialInterface;

  void spawnComponents();

  VCVModule model;
  
  class Aosc3GameModeBase* gameMode;

  // TMap<int, class AVCVParam*> ParamActors;
  TMap<int, class AVCVPort*> InputActors;
  TMap<int, class AVCVPort*> OutputActors;
};