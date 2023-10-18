#pragma once

#include "VCV.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VCVCable.generated.h"

UCLASS()
class OSC3_API AVCVCable : public AActor {
	GENERATED_BODY()
	
public:	
	AVCVCable();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void draw();
  void init(VCVCable model);
  void disconnectFrom(PortIdentity identity);
  void connectTo(PortIdentity identity);
  void setHangingLocation(FVector hangingLocation, FVector hangingForwardVector);
  PortType getHangingType();
  PortIdentity getConnectedPortIdentity();
  int64_t getId();
  VCVCable getModel();
  
private:
  // bool drawn = false;
  VCVCable model;
  float plugOffset = -0.3f;
  float plugRadius = 0.2f;
  float lineWeight = 0.05f;
  FColor cableColor = FColor::MakeRandomColor();
  
  FVector hangingLocation, hangingForwardVector;
  PortType hangingType;
};
