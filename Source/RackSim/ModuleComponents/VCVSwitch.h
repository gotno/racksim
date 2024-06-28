#pragma once

#include "CoreMinimal.h"
#include "ModuleComponents/VCVParam.h"
#include "VCVSwitch.generated.h"

class UTexture2D;
class Aosc3GameModeBase;

UCLASS()
class RACKSIM_API AVCVSwitch : public AVCVParam {
	GENERATED_BODY()

public:
  AVCVSwitch();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
  void Init(VCVParam* vcv_param) override;

  UFUNCTION()
  void SetTexture(FString Filepath, UTexture2D* inTexture);

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* MeshComponent;
  
  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_switch_faced.unit_switch_faced'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_transparent.generic_transparent'");
  TCHAR* FaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face.texture_face'");
  
  UPROPERTY()
  TArray<UTexture2D*> Frames;
  
  Aosc3GameModeBase* GameMode;

public:
  virtual void Engage();
  void SetFrame();
  int GetFrameFromValue();
  // virtual void Alter(float amount);
  // virtual void Release();
  virtual void ResetValue();

  void Update(VCVParam& vcv_param) override;
};
