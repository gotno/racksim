#pragma once

#include "CoreMinimal.h"
#include "ModuleComponents/VCVParam.h"
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
  
  float LastValue;
  float AlterRatio = 0.8f;
  
  FVector LastControllerPosition;
  float LastPositionDelta{0.f};

  // unit vector direction to move
  FVector GetSliderDirectionVector();
  // max move distance
  float MaxOffset;
  // offset to track dragged position vs snap position
  float ShadowOffset;
  // actual offset from zero in world
  float WorldOffset;

  float getOffsetFromValue();
  float getValueFromOffset();
public:
  void engage(FVector ControllerPosition) override;
  void alter(FVector ControllerPosition) override;
  void release() override;
  void resetValue() override;

  void Update(VCVParam& Param) override;
};
