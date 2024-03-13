#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

#include "CableEnd.generated.h"

class AVCVCable;
class AVCVPort;

class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UCapsuleComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDestinationPortTargetedSignature, AVCVPort* /* DestinationPort */);

UCLASS()
class OSC3_API ACableEnd : public AActor {
	GENERATED_BODY()
	
public:	
	ACableEnd();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  AVCVPort* ConnectedPort{nullptr};
  void Connect(AVCVPort* Port);
  void Disconnect();
  void HandleDisconnected();
  bool IsConnected() { return ConnectedPort != nullptr; }
  void Drop();

  AVCVPort* SnapToPort{nullptr};
  
  void SetColor(FColor Color);
  UStaticMeshComponent* GetMesh() { return StaticMeshComponent; }
  AVCVCable* Cable;
  PortType GetType();
  AVCVPort* GetPort();
  AVCVPort* GetConnectedPort();
  
  void SetPosition(FVector Location, FRotator Rotation);
  void UpdatePosition();

  void HandleCableTargeted(AActor* CableEnd, EControllerHand Hand);
  void HandleCableHeld(AActor* CableEnd, EControllerHand Hand);
private:
  TCHAR* JackMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/jack.jack'");
  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");

  UPROPERTY(VisibleAnywhere)
  USceneComponent* SceneComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;

  UPROPERTY(VisibleAnywhere)
  UCapsuleComponent* Collider;
  UFUNCTION()
  void HandleColliderOverlapStart(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  UFUNCTION()
  void HandleColliderOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

  float ColliderCapsuleRadius{0.15f * RENDER_SCALE};
  float ColliderCapsuleHalfHeight{0.4f * RENDER_SCALE};
  FVector ColliderOffset{0.f, 0.f, -ColliderCapsuleHalfHeight};

  void RealignMesh();
  void OffsetMeshFrom(AActor* Actor);
  float MeshOffset{0.2f * RENDER_SCALE};

  bool bHeldByLeftHand{false};
  bool bHeldByRightHand{false};
  bool IsHeld() { return bHeldByLeftHand || bHeldByRightHand; }
  
  void SetSnapToPort(AVCVPort* Port);

public:
  // delegate stuff
  FOnDestinationPortTargetedSignature OnDestinationPortTargetedDelegate;
};
