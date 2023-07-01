#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LED.generated.h"

UCLASS()
class OSC3_API ALED : public AActor {
	GENERATED_BODY()
	
public:	
	ALED();

  USceneComponent* root;
  class UPointLightComponent* light;
  
  void SetModel(struct VCVLight* model);
  void SetColor(FColor color);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  
private:
  VCVLight* model;
};
