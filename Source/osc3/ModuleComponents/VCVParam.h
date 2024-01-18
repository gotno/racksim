#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "VCVParam.generated.h"

class Aosc3GameModeBase;

UCLASS()
class OSC3_API AVCVParam : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVParam();

protected:
	virtual void BeginPlay() override;
  virtual void spawnLights(USceneComponent* attachTo);
  FString getModuleBrand();
  bool engaged{false};
  
private:
  class AVCVModule* owner;
  
  float alterRatio{1.f};

  mutable FCriticalSection DataGuard;

  Aosc3GameModeBase* GameMode;
public:	
	virtual void Tick(float DeltaTime) override;

  virtual void init(struct VCVParam* model);
  virtual void Update(VCVParam& param);
  VCVParam* model;

  void UpdateDisplayValue(const FString& DisplayValue);

  virtual void setValue(float newValue);
  
  virtual void engage();
  virtual void engage(float _value);
  virtual void engage(FVector _location);
  virtual void alter(float amount);
  virtual void alter(FVector _location);
  virtual void release();
  virtual void resetValue();
  
  void GetTooltipText(FString& Label, FString& DisplayValue);
};
