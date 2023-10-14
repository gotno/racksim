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
  virtual void spawnLights(USceneComponent* attachTo);
  FString getModuleBrand();
  
private:
  class AVCVModule* owner;
  
  bool engaged = false;
  float alterRatio = 1.f;

  FRWLock DataGuard;
public:	
	virtual void Tick(float DeltaTime) override;

  virtual void init(struct VCVParam* model);
  VCVParam* model;

  void UpdateDisplayValue(const FString& DisplayValue);

  virtual void setValue(float newValue);
  
  virtual void engage();
  virtual void engage(float _value);
  virtual void engage(FVector _location);
  virtual void alter(float amount);
  virtual void alter(FVector _location);
  virtual void release();
  
  void GetTooltipText(FString& Label, FString& DisplayValue);
};
