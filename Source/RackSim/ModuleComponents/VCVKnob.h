#pragma once

#include "CoreMinimal.h"
#include "ModuleComponents/VCVParam.h"
#include "VCVData/VCV.h"
#include "VCVKnob.generated.h"

class UTexture2D;
class Aosc3GameModeBase;

UCLASS()
class RACKSIM_API AVCVKnob : public AVCVParam {
	GENERATED_BODY()
    
public:
  AVCVKnob();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
  void Init(struct VCVParam* vcv_param) override;

  UFUNCTION()
  void SetTexture(FString Filepath, UTexture2D* inTexture);

private:
  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_knob_faced.unit_knob_faced'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'");
  TCHAR* FaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face3x.texture_face3x'");

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;

  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  UPROPERTY()
  UTexture2D* TextureBackground;
  UPROPERTY()
  UTexture2D* Texture;
  UPROPERTY()
  UTexture2D* TextureForeground;
  
  Aosc3GameModeBase* GameMode;
  
  // TODO: user override of smoothing constant and ratio
  float AlterRatio{2.f};
  float MovingAverageWeight{0.2f};

  FRotator GetRotationFromValue();
  float GetValueFromRotation();
  void UpdateRotation(FRotator inRotation);
  
  FRotator ShadowRotation;
  float LastControllerRoll;
  float LastDeltaRoll;
  
  // gap between min and max position outside of knob range
  float Gap{0};

public:
  void Engage(float ControllerRoll) override;
  void Alter(float ControllerRoll) override;
  void Release() override;
  void ResetValue() override;

  void Update(VCVParam& param) override;
};
