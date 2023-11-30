#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVLight.generated.h"

class Aosc3GameModeBase;
class UTexture2D;

UCLASS()
class OSC3_API AVCVLight : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVLight();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void init(struct VCVLight* model);
  void SetColor(FLinearColor color);
  void SetEmissiveColor(FLinearColor color);
  void SetEmissiveIntensity(float intensity);
  
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

  UFUNCTION()
  void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  
  bool bHandledOverlap{false};

  UTexture2D* texture; 

  Aosc3GameModeBase* gameMode;
  
  VCVLight* model;
};
