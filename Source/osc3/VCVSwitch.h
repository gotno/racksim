#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVSwitch.generated.h"

UCLASS()
class OSC3_API AVCVSwitch : public AVCVParam {
	GENERATED_BODY()

public:
  AVCVSwitch();

protected:
	virtual void BeginPlay() override;

public:
  void init(VCVParam* vcv_param) override;

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
  
  FVector2D handleSize;
  float getOffsetFromValue();
  FVector direction;
  float worldOffset;

public:
  virtual void engage();
  // virtual void alter(float amount);
  // virtual void release();
};
