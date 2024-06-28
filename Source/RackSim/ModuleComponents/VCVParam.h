#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "VCVParam.generated.h"

class Aosc3GameModeBase;
class Aosc3GameState;

UCLASS()
class RACKSIM_API AVCVParam : public AActor {
  GENERATED_BODY()

public:
  AVCVParam();
  friend class AVCVModule;

protected:
  virtual void BeginPlay() override;
  virtual void SpawnLights(USceneComponent* AttachTo);
  FString GetModuleBrand();
  bool bEngaged{false};

  UPROPERTY()
  UMaterialInstanceDynamic* LoadingMaterialInstance;
  UPROPERTY()
  UMaterialInterface* LoadingMaterialInterface;
  TCHAR* LoadingMaterialRef{TEXT("/Script/Engine.Material'/Game/materials/loading.loading'")};

  UPROPERTY()
  UStaticMesh* OverlapMesh;

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
  float GetOverlapDelta();

  virtual void Engage();
  virtual void Engage(float _value);
  virtual void Engage(FVector _location);
  virtual void Alter(float amount);
  virtual void Alter(FVector _location);
  virtual void Release();
  virtual void ResetValue();

  void GetTooltipText(FString& Label, FString& DisplayValue);
};
