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
class OSC3_API AVCVCable : public AActor {
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

  void ConnectToPort(AVCVPort* Port);
  void DisconnectFromPort(PortType Type);
  AVCVPort* GetPort(PortType Type);
  AVCVPort* GetPort(ACableEnd* CableEnd);
  AVCVPort* GetOtherPort(ACableEnd* CableEnd);
  ACableEnd* GetOtherEnd(AVCVPort* Port);
  void Abandon();

  void HandleRegistration();

  // wake for WakeSeconds
  // void Stir(float WakeSeconds = 2.f);
  
  void ToggleLatched() {
    bLatched = !bLatched;
  }

  void SetTension(float inTension);
  void SetOpacity(float Opacity);

  void RecalculatePosition();

  // rack's default cable colors. these will be replaced with
  // colors from settings.json after rackman->Init grabs them
  static inline TArray<FColor> CableColors{
    FColor::FromHex(FString("#f3374b")),
    FColor::FromHex(FString("#ffb437")),
    FColor::FromHex(FString("#00b56e")),
    FColor::FromHex(FString("#3695ef")),
    FColor::FromHex(FString("#8b4ade"))
  };
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;

  // UPROPERTY(VisibleAnywhere)
  // UCableComponent* CableComponent;
  // TCHAR* CableMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/cable.cable'");
  // UPROPERTY()
  // UMaterialInstanceDynamic* CableMaterialInstance;
  // UPROPERTY()
  // UMaterialInterface* CableMaterialInterface;
  UNiagaraSystem* CableFXSystem;
  UNiagaraComponent* CableFXComponent;

  UPROPERTY()
  ACableEnd* CableEndA{nullptr};
  UPROPERTY()
  ACableEnd* CableEndB{nullptr};

  FColor CableColor;
  static inline int CurrentCableColorIndex{0};

  // FTimerHandle CableSleepHandle;
  // void Sleep();
  // void Wake();
  
  Aosc3GameModeBase* GameMode{nullptr};
  
  bool IsComplete();
  bool IsRegistered();
  
  // allowed to exist even if unattached (IsIncomplete)
  bool bLatched{false};
  float Tension{0.f};
};
