#pragma once

#include "CoreMinimal.h"
#include "ModuleComponents/VCVParam.h"
#include "VCV.h"
#include "VCVKnob.generated.h"

class UTexture2D;
class Aosc3GameModeBase;

UCLASS()
class OSC3_API AVCVKnob : public AVCVParam {
	GENERATED_BODY()
    
public:
  AVCVKnob();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
  void init(struct VCVParam* vcv_param) override;

private:
  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_knob_faced.unit_knob_faced'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'");
  TCHAR* FaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face3x.texture_face3x'");

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  UPROPERTY()
  UTexture2D* textureBackground;
  UPROPERTY()
  UTexture2D* texture;
  UPROPERTY()
  UTexture2D* textureForeground;
  
  Aosc3GameModeBase* gameMode;
  
  // TODO: user override of smoothing constant and ratio
  float alterRatio{1.f};
  float MovingAverageWeight{0.2f};

  FRotator getRotationFromValue();
  float getValueFromRotation();
  void updateRotation(FRotator newRotation);
  
  FRotator shadowRotation;
  float LastControllerRoll;
  float LastDeltaRoll;
  
  // gap between min and max position outside of knob range
  float gap{0};

public:
  void engage(float ControllerRoll) override;
  void alter(float ControllerRoll) override;
  void release() override;
  void resetValue() override;

  void Update(VCVParam& param) override;
};
