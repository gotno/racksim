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
  TCHAR* CircleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_led_round_faced.unit_led_round_faced'");
  TCHAR* RectangleMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_led_rect_faced.unit_led_rect_faced'");

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/led.led'");
  TCHAR* FaceMaterialReference = TEXT("/Script/Engine.Material'/Game/meshes/faced/led_svg.led_svg'");

  UFUNCTION()
  void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  
  bool bHandledOverlap{false};

  UTexture2D* texture; 

  Aosc3GameModeBase* gameMode;
  
  VCVLight* model;
};
