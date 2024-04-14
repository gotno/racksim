#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "VCVParam.generated.h"

class Aosc3GameModeBase;
class Aosc3GameState;

UCLASS()
class OSC3_API AVCVParam : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVParam();

protected:
	virtual void BeginPlay() override;
  virtual void SpawnLights(USceneComponent* AttachTo);
  FString GetModuleBrand();
  bool bEngaged{false};
  
private:
  class AVCVModule* Module;
  
  float AlterRatio{1.f};

  mutable FCriticalSection DataGuard;

  Aosc3GameModeBase* GameMode;
  Aosc3GameState* GameState;
  float OldValue;
public:	
	virtual void Tick(float DeltaTime) override;

  virtual void Init(struct VCVParam* Model);
  virtual void Update(VCVParam& Param);
  VCVParam* Model;

  void UpdateDisplayValue(const FString& DisplayValue);

  virtual void SetValue(float inValue);
  
  virtual void Engage();
  virtual void Engage(float _value);
  virtual void Engage(FVector _location);
  virtual void Alter(float amount);
  virtual void Alter(FVector _location);
  virtual void Release();
  virtual void ResetValue();
  
  void GetTooltipText(FString& Label, FString& DisplayValue);
};
