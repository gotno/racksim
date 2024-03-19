#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModuleWeldment.generated.h"

class AVCVModule;

UCLASS()
class OSC3_API AModuleWeldment : public AActor {
	GENERATED_BODY()
	
public:	
	AModuleWeldment();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
  void AddModuleBack(AVCVModule* Module);
  void AddModuleFront(AVCVModule* Module);

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;

  UPROPERTY()
  TArray<AVCVModule*> Modules;

  FVector GrabOffset;
  FVector LastLocationDelta;
  FVector LastGrabbedLocation;
  FRotator LastGrabbedRotation;

  void AddModule(AVCVModule* Module);
  void ValidateModuleInclusion(AVCVModule* Module);

  void ResetLocation();
  void HighlightModules();
  void UnhighlightModules();
};
