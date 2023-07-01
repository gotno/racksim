#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVKnob.generated.h"

UCLASS()
class OSC3_API AVCVKnob : public AVCVParam {
	GENERATED_BODY()
    
public:
  AVCVKnob();

protected:
	virtual void BeginPlay() override;

public:
  void init(struct VCVParam* vcv_param) override;

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* MarkerMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* MarkerMaterialInterface;

  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_knob.unit_knob'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/knob_base.knob_base'");
  TCHAR* MarkerMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/knob_marker.knob_marker'");
  
  float lastAmount = 0.f;
  float alterRatio = 1.f / 2.f;

  FRotator getRotationFromValue();
  float getValueFromRotation();
  
  FRotator shadowRotation;

public:
  void engage() override;
  void alter(float amount) override;
  void release() override;
};
