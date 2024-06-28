#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVDisplay.generated.h"

UCLASS()
class RACKSIM_API AVCVDisplay : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVDisplay();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  void Init(struct VCVDisplay* vcv_display);

private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* MaterialInstance;

  UPROPERTY()
  UMaterialInterface* MaterialInterface;
  
  VCVDisplay* Model;
};
