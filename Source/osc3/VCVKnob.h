#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
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
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/knob_base.knob_base'");
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
  
  // float lastAmount = 0.f;
  float alterRatio = 1.f;

  FRotator getRotationFromValue();
  float getValueFromRotation();
  void updateRotation(FRotator newRotation);
  
  FRotator shadowRotation;
  float LastControllerRoll;
  float LastDeltaRoll;

public:
  void engage(float ControllerRoll) override;
  void alter(float ControllerRoll) override;
  void release() override;
};
