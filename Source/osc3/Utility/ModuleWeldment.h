#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "osc3.h"

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
  
  void AddModuleBack(AVCVModule* Module);
  void AddModuleFront(AVCVModule* Module);

  void Append(AModuleWeldment* OtherWeldment);
  void Prepend(AModuleWeldment* OtherWeldment);
  bool SplitIfAdjacent(AVCVModule* leftModule, AVCVModule* rightModule);

  void GetModules(TArray<AVCVModule*>& outModules);
  void GetModuleIds(TArray<int64>& outModuleIds);
  bool Contains(AActor* Actor);
  int IndexOf(AActor* Actor);

  void InitSnapMode();
  void CancelSnapMode();
  bool IsInSnapMode();
  // align all modules with module that is snapping
  void FollowSnap(AVCVModule* Module);

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
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

  void AddModule(AVCVModule* Module);
  void ValidateModuleInclusion(AVCVModule* Module);
  void PositionModule(int32 ModuleIndex, int32 RelativeToIndex);

  void ResetLocation();
  void HighlightModules();
  void UnhighlightModules();
};
