#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

#include "GrabbableActor.generated.h"

UCLASS()
class OSC3_API AGrabbableActor : public AActor
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

protected:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
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

public:
  // delegate stuff
  void HighlightIfTargeted(AActor* GrabbableTarget, EControllerHand Hand);
};
