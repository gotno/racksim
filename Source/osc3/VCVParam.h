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
  virtual void spawnLights(USceneComponent* attachTo, FVector offset);
  
private:
  class AVCVModule* owner;
  
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
