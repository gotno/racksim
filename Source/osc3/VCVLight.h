#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVLight.generated.h"

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
  
  UFUNCTION()
  void onBeginOverlap(
    class UPrimitiveComponent* OverlappedComp,
    class AActor* OtherActor,
    class UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
  );
  
private:
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
  UStaticMeshComponent* BaseMeshComponent;
  
  UPROPERTY()
  UStaticMesh* StaticMesh;
  
  UPROPERTY()
  UMaterialInstanceDynamic* MaterialInstance;

  UPROPERTY()
  UMaterialInterface* MaterialInterface;
  
  UPROPERTY()
  UStaticMesh* circleMesh;
  
  UPROPERTY()
  UStaticMesh* rectangleMesh;
  
  VCVLight* model;
};
