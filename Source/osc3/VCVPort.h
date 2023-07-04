#pragma once

#include "VCV.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVPort.generated.h"

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
  PortIdentity getIdentity();
  bool canConnect(PortType type);

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* BodyMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* HoleMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BodyMaterialInterface;
  UPROPERTY()
  UMaterialInterface* HoleMaterialInterface;
  
  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_port.unit_port'");
  TCHAR* BodyMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/port_body.port_body'");
  TCHAR* HoleMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/port_hole.port_hole'");
  
  VCVPort* model;
  TArray<int64_t> cableIds;
};