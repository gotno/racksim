#pragma once

#include "VCV.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVModule.generated.h"

class UTexture2D;
class Aosc3GameModeBase;
class AVCVLight;
class AVCVPort;
class AVCVParam;

UCLASS()
class OSC3_API AVCVModule : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVModule();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void init(VCVModule model);
  
  void GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector);
  void AttachCable(const PortIdentity& identity, int64_t cableId);
  void DetachCable(const PortIdentity& identity, int64_t cableId);
  void UpdateLight(int32 lightId, FLinearColor color);
  AVCVParam* GetParamActor(const int& paramId) { return ParamActors[paramId]; }

  UPROPERTY()
  TMap<int32, AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, AVCVLight*> ParamLightActors;
  
  void registerParamLight(int64_t lightId, AVCVLight* lightActor);
  void paramUpdated(int32 paramId, float value);

  FString getBrand();

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void ReleaseGrab();
  
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY()
  UStaticMesh* StaticMesh;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  UPROPERTY()
  UTexture2D* texture;

  void spawnComponents();

  VCVModule model;
  
  Aosc3GameModeBase* gameMode;
  
  UFUNCTION()
  void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherCompomponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  UFUNCTION()
  void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherCompomponent, int32 OtherBodyIndex);
  void SetHighlighted(bool bHighlighted);

  bool bGrabEngaged{false};
  FVector GrabOffset;
  FVector LastGrabbedLocation;
  FRotator LastGrabbedRotation;
  FVector LastLocationDelta;

  TMap<int, AVCVParam*> ParamActors;
  TMap<int, AVCVPort*> InputActors;
  TMap<int, AVCVPort*> OutputActors;
};