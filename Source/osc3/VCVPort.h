#pragma once

#include "osc3.h"
#include "VCV.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVPort.generated.h"

class Aosc3GameModeBase;
class UTexture2D;

class AVCVCable;

UCLASS()
class OSC3_API AVCVPort : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVPort();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  void init(struct VCVPort* model);
  
  void addCableId(int64_t cableId);
  bool getCableId(int64_t& cableId);
  void removeCableId(int64_t cableId);
  PortIdentity getIdentity();
  bool CanConnect(PortType Type);
  bool hasCables();

  void GetTooltipText(FString& Name, FString& Description);

  PortType Type;
  void AddCable(AVCVCable* Cable);
  void RemoveCable(AVCVCable* Cable);
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
  
  Aosc3GameModeBase* gameMode;

  VCVPort* model;
  TArray<int64_t> cableIds;

  UPROPERTY()
  TArray<AVCVCable*> Cables;
  
  UPROPERTY()
  TArray<AVCVCable*> AttachedCableActors;
};
