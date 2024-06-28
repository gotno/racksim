#pragma once

#include "CoreMinimal.h"
#include "ModuleComponents/VCVParam.h"
#include "VCVSlider.generated.h"

class Aosc3GameModeBase;
class UTexture2D;

UCLASS()
class RACKSIM_API AVCVSlider : public AVCVParam {
  GENERATED_BODY()

public:
  AVCVSlider();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;
  void Init(VCVParam* vcv_param) override;

  UFUNCTION()
  void SetTexture(FString Filepath, UTexture2D* inTexture);

  FVector GetHandleLocation() {
    return HandleMeshComponent->GetComponentLocation();
  }

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* HandleMeshComponent;

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
  UTexture2D* BaseTexture;
  UPROPERTY()
  UTexture2D* HandleTexture;

  Aosc3GameModeBase* GameMode;

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
  void Engage(FVector ControllerPosition) override;
  void Alter(FVector ControllerPosition) override;
  void Release() override;
  void ResetValue() override;

  void Update(VCVParam& vcv_param) override;
};
