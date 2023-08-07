#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVSlider.generated.h"

class Aosc3GameModeBase;
class UTexture2D;

UCLASS()
class OSC3_API AVCVSlider : public AVCVParam {
	GENERATED_BODY()
	
public:
  AVCVSlider();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
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
  UMaterialInstanceDynamic* BaseFaceMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* HandleMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* HandleFaceMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* BaseFaceMaterialInterface;
  UPROPERTY()
  UMaterialInterface* HandleMaterialInterface;
  UPROPERTY()
  UMaterialInterface* HandleFaceMaterialInterface;

  TCHAR* BaseMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_slider_base_faced.unit_slider_base_faced'");
  TCHAR* HandleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_switch_handle_faced.unit_switch_handle_faced'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_transparent.generic_transparent'");
  TCHAR* BaseFaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face.texture_face'");
  TCHAR* HandleMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");
  TCHAR* HandleFaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face.texture_face'");

  UPROPERTY()
  UTexture2D* baseTexture;
  UPROPERTY()
  UTexture2D* handleTexture;

  Aosc3GameModeBase* gameMode;
  
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
  void spawnLights(USceneComponent* attachTo, FVector offset) override;

public:
  virtual void engage();
  virtual void alter(float amount);
  virtual void release();
};
