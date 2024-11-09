#pragma once

#include "CoreMinimal.h"

#include "osc3.h"

#include "VCVCable.generated.h"

class USceneComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UCableComponent;

class Aosc3GameModeBase;
class AVCVPort;
class ACableEnd;

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class RACKSIM_API AVCVCable : public AActor {
  GENERATED_BODY()

public:
  AVCVCable();

protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  virtual void Tick(float DeltaTime) override;

  int64_t Id{-1};
  void SetId(int64_t inId) { Id = inId; }
  void ResetId() { SetId(-1); }

  void ConnectToPort(AVCVPort* Port);
  void DisconnectFromPort(PortType Type);
  AVCVPort* GetPort(PortType Type);
  AVCVPort* GetPort(ACableEnd* CableEnd);
  AVCVPort* GetOtherPort(ACableEnd* CableEnd);
  ACableEnd* GetOtherEnd(AVCVPort* Port);
  void SetEndPositions(
    const FVector& EndALocation,
    const FRotator& EndARotation,
    const FVector& EndBLocation,
    const FRotator& EndBRotation
  );
  void GetEndPositions(
    FVector& EndALocation,
    FRotator& EndARotation,
    FVector& EndBLocation,
    FRotator& EndBRotation
  );
  void Abandon();

  void HandleRegistration();

  void ToggleLatched() {
    bLatched = !bLatched;
    SetColor(CableColor);
  }
  bool IsLatched() {
    return bLatched;
  }

  bool IsComplete();

  void SetColor(FColor Color);
  FColor GetColor() {
    return CableColor;
  }
  void CycleColor(int Direction);
  void CycleColor();
  static inline int CableColorCycleDirection{1};

  void UpdateCable();

  // rack's default cable colors. these will be replaced with
  // colors from settings.json after rackman->Init grabs them
  static inline TArray<FColor> CableColors{
    FColor::FromHex(FString("#f3374b")),
    FColor::FromHex(FString("#ffb437")),
    FColor::FromHex(FString("#00b56e")),
    FColor::FromHex(FString("#3695ef")),
    FColor::FromHex(FString("#8b4ade"))
  };
  static inline int CurrentCableColorIndex{-1};

  static inline float Tension{DEFAULT_CABLE_TENSION * 0.01f};
  static void SetTension(float Percent) {
    Tension = Percent * 0.01f;
  }

  static inline float Opacity{DEFAULT_CABLE_OPACITY * 0.01f};
  static void SetOpacity(float Percent) {
    Opacity = Percent * 0.01f;
  }
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;

  UNiagaraSystem* CableFXSystem;
  UNiagaraComponent* CableFXComponent;

  UPROPERTY()
  ACableEnd* CableEndA{nullptr};
  UPROPERTY()
  ACableEnd* CableEndB{nullptr};

  FColor CableColor;

  Aosc3GameModeBase* GameMode{nullptr};

  bool IsRegistered();

  // allowed to exist even if unattached
  bool bLatched{false};

public:
  static inline float Scale{1.f};
  void Rescale();
};
