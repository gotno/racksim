#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVParam.generated.h"

UCLASS()
class OSC3_API AVCVParam : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVParam();

protected:
	virtual void BeginPlay() override;
  
private:
  class AVCVModule* owner;
  
  FVector lightOffset;
  void SpawnLights(USceneComponent* attachTo);

  bool engaged = false;
  float alterRatio = 1.f;

public:	
	virtual void Tick(float DeltaTime) override;

  virtual void init(struct VCVParam* model);
  VCVParam* model;

  virtual void setValue(float newValue);
  
  virtual void engage();
  virtual void alter(float amount);
  virtual void release();
};
