#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVSlider.generated.h"

UCLASS()
class OSC3_API AVCVSlider : public AVCVParam {
	GENERATED_BODY()
	
public:
  AVCVSlider();

protected:
	virtual void BeginPlay() override;

public:
  void setModel(VCVParam* vcv_param) override;

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* HandleMeshComponent;
  
  UPROPERTY()
  UStaticMesh* BaseStaticMesh;
  UPROPERTY()
  UStaticMesh* HandleStaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* HandleMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* HandleMaterialInterface;

  TCHAR* BaseMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_switch_base.unit_switch_base'");
  TCHAR* HandleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_switch_handle.unit_switch_handle'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");
  TCHAR* HandleMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");
  
  float lastAlterAmount = 0.f;
  float lastValue;
  float alterRatio = 1.f / 128.f;
  
  // unit vector direction to move, updated every `engage`
  FVector direction;
  // max move distance
  float minMaxOffsetDelta;
  // offset to track dragged position vs snap position
  float shadowOffset;
  // actual offset from zero in world
  float worldOffset;

  float getOffsetFromValue();
  float getValueFromOffset();
  
  FVector lightOffset{-0.11f, 0, 0};

public:
  virtual void engage();
  virtual void alter(float amount);
  virtual void release();
};
