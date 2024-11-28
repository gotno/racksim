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
class RACKSIM_API ACableEnd : public AActor {
  GENERATED_BODY()

  friend class AVCVCable;

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
  // drops the held cable end and returns whether it was connected to a port
  bool Drop();

  AVCVPort* SnapToPort{nullptr};

  void SetColor(FColor Color);
  AVCVCable* Cable;
  PortType GetType();
  AVCVPort* GetPort();
  AVCVPort* GetConnectedPort();

  void GetPosition(FVector& Location, FRotator& Rotation);
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
  UPROPERTY()
  UMaterialInstanceDynamic* RingMaterialInstance;

  UPROPERTY(VisibleAnywhere)
  UCapsuleComponent* Collider;
  UFUNCTION()
  void HandleColliderOverlapStart(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
  UFUNCTION()
  void HandleColliderOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

  float ColliderCapsuleRadius{0.15f};
  float ColliderCapsuleHalfHeight{0.4f};
  FVector ColliderOffset{0.f, 0.f, -ColliderCapsuleHalfHeight};

  FDelegateHandle hPortTransformUpdated;

  UStaticMeshComponent* GetMesh() { return StaticMeshComponent; }

  void RealignMesh();
  void OffsetMeshFrom(AActor* Actor);
  float GetMeshOffset();

  bool bHeldByLeftHand{false};
  bool bHeldByRightHand{false};
  bool IsHeld() { return bHeldByLeftHand || bHeldByRightHand; }

  void SetSnapToPort(AVCVPort* Port);
  void HandlePortTransformUpdated(
    USceneComponent* _UpdatedComponent,
    EUpdateTransformFlags _UpdateTransformFlags,
    ETeleportType _Teleport
  );

public:
  // delegate stuff
  FOnDestinationPortTargetedSignature OnDestinationPortTargetedDelegate;
};
