#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVSwitch.generated.h"

class UTexture2D;
class Aosc3GameModeBase;

UCLASS()
class OSC3_API AVCVSwitch : public AVCVParam {
	GENERATED_BODY()

public:
  AVCVSwitch();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
  void init(VCVParam* vcv_param) override;

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* MeshComponent;
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

  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_switch_faced.unit_switch_faced'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");
  TCHAR* FaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face.texture_face'");
  
  UPROPERTY()
  TArray<UTexture2D*> frames;
  
  Aosc3GameModeBase* gameMode;

public:
  virtual void engage();
  // virtual void alter(float amount);
  // virtual void release();
};
