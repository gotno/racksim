#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// #include "VCV.h"

#include "VCVCable.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
// class UCableComponent;

class AVCVPort;
// class AVCVModule;

enum PortType {
  Input, Output
};

UCLASS()
class OSC3_API AVCVCable : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVCable();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void SetPort(AVCVPort* Port);

  // void draw();
  // void init(VCVCable model);
  // void disconnectFrom(PortIdentity identity);
  // void connectTo(PortIdentity identity);
  // void setHangingLocation(FVector hangingLocation, FVector hangingForwardVector);
  // PortType getHangingType();
  // PortIdentity getConnectedPortIdentity();
  // void setId(int64_t& inId);
  // int64_t getId();
  // VCVCable getModel();
  
  // void SetAlive(bool inAlive);
  // void Sleep();
  // void Wake();
  
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

  // VCVCable model;
  // float plugOffset = -0.3f;
  // float plugRadius = 0.2f;
  // float lineWeight = 0.05f;
  FColor cableColor;
  TArray<FColor> cableColors{
    FColor::FromHex(FString("#F3374B")),
    FColor::FromHex(FString("#ffb437")),
    FColor::FromHex(FString("#00b56e")),
    FColor::FromHex(FString("#3695ef")),
    FColor::FromHex(FString("#8b4ade"))
  };
  
  // FVector hangingLocation, hangingForwardVector;
  // PortType hangingType;
  
  // bool bAlive{false};
  

  // // THE NEW SHIT
  // AVCVPort* InputPortActor;
  // AVCVModule* InputModuleActor;
  // AVCVPort* OutputPortActor;
  // AVCVModule* OutputModuleActor;
  
  // bool IsComplete() {
  //   return InputPortActor && OutputPortActor;
  // }
  
  // // allowed to exist even if incomplete (hanging end is floating, not held)
  // bool Latched{false};
  UPROPERTY()
  TMap<PortType, AVCVPort*> Ports;
};
