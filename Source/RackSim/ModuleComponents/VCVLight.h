#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "VCVLight.generated.h"

class Aosc3GameModeBase;
class UTexture2D;

UCLASS()
class RACKSIM_API AVCVLight : public AActor {
  GENERATED_BODY()

public:
  AVCVLight();
  friend class AVCVModule;

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;

  void Init(struct VCVLight* Model);
  void SetColor(FLinearColor Color);
  void SetEmissiveColor(FLinearColor Color);
  void SetEmissiveIntensity(float Intensity);
  
private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  UPROPERTY()
  UStaticMesh* circleMesh;
  UPROPERTY()
  UStaticMesh* rectangleMesh;
  TCHAR* CircleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_led_round_lens.unit_led_round_lens'");
  TCHAR* RectangleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_led_rectangle.unit_led_rectangle'");

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/transparent_led.transparent_led'");

  void HandleOverlap();

  Aosc3GameModeBase* GameMode;
  
  VCVLight* Model;
};
