#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ModuleWeldment.generated.h"

class Aosc3GameModeBase;
class AVCVModule;
class UBoxComponent;

UCLASS()
class OSC3_API AModuleWeldment : public AActor {
	GENERATED_BODY()
	
public:	
	AModuleWeldment();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
  virtual void Tick(float DeltaTime) override;
  void SnapModeTick();
  
  void AddModuleBack(AVCVModule* Module);
  void AddModuleFront(AVCVModule* Module);

  void Append(AModuleWeldment* OtherWeldment);

  void GetModules(TArray<AVCVModule*>& outModules);
  void GetModuleIds(TArray<int64>& outModuleIds);

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void ReleaseGrab();
private:
  Aosc3GameModeBase* GameMode{nullptr};

  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;

  UPROPERTY()
  TArray<AVCVModule*> Modules;

  FVector GrabOffset;
  FVector LastLocationDelta;
  FVector LastGrabbedLocation;
  FRotator LastGrabbedRotation;

  UBoxComponent* SnapToSide{nullptr};
  void Snap(bool bTemporarily = true);
  void Unsnap();

  void AddModule(AVCVModule* Module);
  void ValidateModuleInclusion(AVCVModule* Module);

  void ResetLocation();
  void HighlightModules();
  void UnhighlightModules();
};