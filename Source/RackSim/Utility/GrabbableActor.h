#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

#include "GrabbableActor.generated.h"

class AModuleWeldment;

UCLASS()
class RACKSIM_API AGrabbableActor : public AActor
{
  GENERATED_BODY()

public:
  AGrabbableActor();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;

  virtual void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  virtual void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  virtual void ReleaseGrab();
  virtual void SetHighlighted(bool bHighlighted, FLinearColor OutlineColor = OUTLINE_COLOR);

  void Summon(FVector Location, FRotator Rotation, bool bInstant = false);

  bool IsInWeldment() { return !!Weldment; }
  AModuleWeldment* GetWeldment() {
    return Weldment;
  }
protected:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
  UPROPERTY(VisibleAnywhere)
  USceneComponent* StaticMeshRoot;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* StaticMeshComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* OutlineMeshComponent;

  UPROPERTY()
  UMaterialInstanceDynamic* OutlineMaterialInstance;
  UPROPERTY()
  UMaterialInterface* OutlineMaterialInterface;

  bool bTargetGrabOfLeftHand{false};
  bool bTargetGrabOfRightHand{false};

  bool bGrabEngaged{false};
  FVector GrabOffset;
  FVector LastLocationDelta;
  FVector LastGrabbedLocation;
  FRotator LastGrabbedRotation;

  AModuleWeldment* Weldment{nullptr};

  void ResetMeshPosition();
  void CenterActorOnMesh();
  virtual void HandleSummons(float DeltaTime);

  // summoning
  FVector StartLocation, TargetLocation;
  FRotator StartRotation, TargetRotation;
  float SummonAlpha{0.f};
  bool bSummoned;
  float SummonDuration{0.4f}; // seconds
public:
  // delegate stuff
  void HighlightIfTargeted(AActor* GrabbableTarget, EControllerHand Hand);
};
