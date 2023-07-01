#pragma once

#include "CoreMinimal.h"
#include "VCVParam.h"
#include "VCVButton.generated.h"

UCLASS()
class OSC3_API AVCVButton : public AVCVParam {
	GENERATED_BODY()
    
public:
  AVCVButton();

protected:
	virtual void BeginPlay() override;

public:
  void setModel(VCVParam* vcv_param) override;

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* MaterialInstance;

  UPROPERTY()
  UMaterialInterface* MaterialInterface;

  TCHAR* MeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_button.unit_button'");
  TCHAR* MaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");
  
  FVector lightOffset{-0.11f, 0, 0};
public:
  void engage() override;
  void release() override;
};
