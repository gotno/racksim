#pragma once

#include "VCVData/VCV.h"
#include "Utility/GrabbableActor.h"

#include "VCVModule.generated.h"

class Aosc3GameModeBase;
class AVCVLight;
class AVCVPort;
class AVCVParam;
class AContextMenu;
class AModuleWeldment;

class UTexture2D;
class UBoxComponent;

UCLASS()
class OSC3_API AVCVModule : public AGrabbableActor {
	GENERATED_BODY()
	
public:	
	AVCVModule();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

  void Init(VCVModule vcv_module, TFunction<void ()> ReadyCallback);
  
  void GetSlugs(FString& PluginSlug, FString& Slug);
  int64_t Id{-1};

  void GetModulePosition(FVector& Location, FRotator& Rotation);
  void GetModuleLandingPosition(FVector& Location, FRotator& Rotation, bool bOffset = true);

  void UpdateLight(int32 LightId, FLinearColor Color);
  AVCVParam* GetParamActor(const int& ParamId) { return ParamActors[ParamId]; }
  AVCVPort* GetPortActor(PortType Type, int32& PortId);

  void ToggleContextMenu();

  void TriggerCableUpdates();

  void SetSnapMode(bool inbSnapMode);

  UPROPERTY()
  TMap<int32, AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, AVCVLight*> ParamLightActors;
  
  void RegisterParamLight(int64_t LightId, AVCVLight* LightActor);
  void ParamUpdated(int32 ParamId, float Value);

  FString Brand;
  FString Name;

  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;

  void SetWeldment(AModuleWeldment* inWeldment) {
    Weldment = inWeldment;
  }
  bool IsInWeldment() {
    return !!Weldment;
  }
private:
  UPROPERTY()
  UBoxComponent* SnapColliderLeft;
  UPROPERTY()
  UBoxComponent* SnapColliderRight;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;
  UPROPERTY()
  UMaterialInstanceDynamic* FaceMaterialInstance;
  UPROPERTY()
  UMaterialInterface* FaceMaterialInterface;

  UPROPERTY()
  UTexture2D* Texture;
  
  UPROPERTY()
  AContextMenu* ContextMenu;

  void SpawnComponents();

  VCVModule Model;

  void SnapModeTick();
  bool bSnapMode{false};
  
  Aosc3GameModeBase* GameMode;

  TMap<int, AVCVParam*> ParamActors;
  TMap<int, AVCVPort*> InputActors;
  TMap<int, AVCVPort*> OutputActors;
};