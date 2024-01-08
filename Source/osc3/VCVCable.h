#pragma once

#include "CoreMinimal.h"

// #include "VCV.h"
#include "osc3.h"

#include "VCVCable.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
// class UCableComponent;

class Aosc3GameModeBase;
class AVCVPort;
// class AVCVModule;

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

  void SetPort(AVCVPort* Port);
  void UnsetPort(PortType Type);
  AVCVPort* GetPort(PortType Type);

  void UpdateEndPositions();
  void SetHangingEndLocation(FVector inHangingLocation, FVector inHangingForwardVector);
  PortType GetHangingType();
  
private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* InputMeshComponent;
  UPROPERTY(VisibleAnywhere)
  UStaticMeshComponent* OutputMeshComponent;
  
  TCHAR* JackMeshReference = TEXT("/Script/Engine.StaticMesh'/Game/meshes/jack.jack'");
  
  // UPROPERTY(VisibleAnywhere)
  // UCableComponent* CableComponent;

  UPROPERTY()
  UMaterialInstanceDynamic* BaseMaterialInstance;
  UPROPERTY()
  UMaterialInterface* BaseMaterialInterface;

  TCHAR* BaseMaterialReference = TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'");

  FColor CableColor;
  // TODO: replace these with colors from potentially-user-overridden rack settings
  TArray<FColor> CableColors{
    FColor::FromHex(FString("#F3374B")),
    FColor::FromHex(FString("#ffb437")),
    FColor::FromHex(FString("#00b56e")),
    FColor::FromHex(FString("#3695ef")),
    FColor::FromHex(FString("#8b4ade"))
  };
  static inline int CurrentCableColorIndex{0};
  
  FVector HangingLocation, HangingForwardVector;
  
  Aosc3GameModeBase* GameMode{nullptr};
  
  bool IsComplete() {
    return Ports.Contains(PortType::Input) && Ports.Contains(PortType::Output);
  }

  bool IsRegistered() {
    return Id != -1;
  }

  void HandlePortChange();
  
  // // allowed to exist even if incomplete (hanging end is floating, not held)
  // bool Latched{false};
  UPROPERTY()
  TMap<PortType, AVCVPort*> Ports;
};
