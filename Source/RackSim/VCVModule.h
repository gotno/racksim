#pragma once

#include "osc3.h"
#include "VCVData/VCV.h"
#include "Utility/GrabbableActor.h"

#include "VCVModule.generated.h"

class Aosc3GameModeBase;
class Aosc3GameState;
class AVCVLight;
class AVCVPort;
class AVCVParam;
class AContextMenu;
class AModuleWeldment;

class UTexture2D;
class UBoxComponent;

// used to remove module from library parking lot
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFirstMovedSignature, AVCVModule* /* this */);

UCLASS()
class RACKSIM_API AVCVModule : public AGrabbableActor {
  GENERATED_BODY()

  friend class AModuleWeldment;

public:
  AVCVModule();

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void HandleSummons(float DeltaTime) override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void Summon(FVector Location, FRotator Rotation, bool bInstant = false) override;

  void Init(VCVModule vcv_module, TFunction<void ()> ReadyCallback);
  UFUNCTION()
  void SetTexture(FString& FilePath, UTexture2D* Texture);
  void SetStagedForDestroy(bool inbStagedForDestroy);
  bool IsStagedForDestroy() { return bStagedForDestroy; };
  
  void GetSlugs(FString& PluginSlug, FString& Slug);
  int64_t Id{-1};

  void GetModulePosition(FVector& Location, FRotator& Rotation);
  void GetModuleLandingPosition(FVector& Location, FRotator& Rotation, bool bOffset = true);

  void UpdateLight(int32 LightId, FLinearColor Color);
  AVCVParam* GetParamActor(const int& ParamId) {
    if (!ParamActors.Contains(ParamId)) return nullptr;
    return ParamActors[ParamId];
  }
  AVCVPort* GetPortActor(PortType Type, int32& PortId);

  void ToggleContextMenu();

  void TriggerCableUpdates();

  void InitSnapMode();
  void CancelSnapMode();
  bool IsInSnapMode(bool bActual = false);

  UPROPERTY()
  TMap<int32, AVCVLight*> LightActors;
  UPROPERTY()
  TMap<int32, AVCVLight*> ParamLightActors;
  
  void RegisterParamLight(int64_t LightId, AVCVLight* LightActor);
  void ParamUpdated(int32 ParamId, float Value);

  FString Brand;
  FString Name;

  void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) override;
  void ReleaseGrab() override;

  void SetWeldment(AModuleWeldment* inWeldment) {
    HandleFirstMoved();
    Weldment = inWeldment;
  }

  // get the location of the edge of this module,
  // the vector pointing from the center through that edge,
  // and the rotation of the module
  void GetAlignToMeshInfo(FSnapModeSide AlignToSide, FVector& outLocation, FVector& outVector, FRotator& outRotation);

  // align the static mesh to the side of another module with optional offset
  void AlignMeshTo(AVCVModule* Module, FSnapModeSide AlignToSide, float Offset = 0.f);

  // fully align the actor to the side of another module
  void AlignActorTo(AVCVModule* Module, FSnapModeSide AlignToSide);
private:
  Aosc3GameState* GameState;
  bool bStagedForDestroy{false};

  UPROPERTY()
  UBoxComponent* SnapColliderLeft;
  UPROPERTY()
  UBoxComponent* SnapColliderRight;

  UPROPERTY()
  UStaticMeshComponent* SnapIndicatorLeft;
  UPROPERTY()
  UStaticMeshComponent* SnapIndicatorLeftReflected;

  UPROPERTY()
  UStaticMeshComponent* SnapIndicatorRight;
  UPROPERTY()
  UStaticMeshComponent* SnapIndicatorRightReflected;

  UPROPERTY()
  UMaterialInstanceDynamic* SnapIndicatorMaterialInstance;
  UPROPERTY()
  UMaterialInterface* SnapIndicatorMaterialInterface;

  UPROPERTY()
  UMaterialInstanceDynamic* LoadingMaterialInstance;
  UPROPERTY()
  UMaterialInterface* LoadingMaterialInterface;
  TCHAR* LoadingMaterialRef{TEXT("/Script/Engine.Material'/Game/materials/loading.loading'")};

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

  void SetSnapMode(FSnapModeSide inSnapModeSide);
  FSnapModeSide SnapModeSide{FSnapModeSide::None};

  void SnapModeTick();
  FHitResult RunRightwardSnapTrace();
  FHitResult RunLeftwardSnapTrace();
  float SnapTraceDistance{3.f};

  // the collider of the module to snap to,
  // from which left/right side can be determined
  UBoxComponent* SnapToSide{nullptr};

  float SnapIndicatorThickness = 0.2f;

  // snap the static mesh to the module owned by SnapToSide
  void SnapMesh();
  // make the snap permanent
  void WeldSnap();

  void HandleFirstMoved();

  Aosc3GameModeBase* GameMode;

  TMap<int, AVCVParam*> ParamActors;
  TMap<int, AVCVPort*> InputActors;
  TMap<int, AVCVPort*> OutputActors;

public:
  static inline float Scale{1.f};
  void Rescale();
  float GetModuleDepth() { return UNSCALED_MODULE_DEPTH * Scale; }
  float GetPanelWidth() { return UnscaledPanelWidth * Scale; }
  float GetPanelHeight() { return UnscaledPanelHeight * Scale; }

private:
  float UnscaledPanelHeight{UNSCALED_MODULE_HEIGHT};
  float UnscaledPanelWidth{0.508f}; // 1hp

// delegate stuff
public:
  FOnFirstMovedSignature OnFirstMoved;
private:
  bool bHasBeenMoved{false};
};