#include "LED.h"
#include "Components/PointLightComponent.h"

#include "VCV.h"

ALED::ALED() {
	PrimaryActorTick.bCanEverTick = true;

  root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
  SetRootComponent(root);

  light = CreateDefaultSubobject<UPointLightComponent>(TEXT("plight"));
  light->SetupAttachment(root);
  light->SetMobility(EComponentMobility::Movable);
  // light->SetWorldLocation(GetActorLocation());
  light->SetCastShadows(false);
  light->SetIntensity(8.f);
  light->SetAttenuationRadius(0.2f);
  light->SetUseInverseSquaredFalloff(false);
  light->SetLightFalloffExponent(0.4f);
}

void ALED::BeginPlay() {
	Super::BeginPlay();
}

void ALED::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void ALED::init(VCVLight* vcv_light) {
  model = vcv_light;
  // light->SetAttenuationRadius(model->box.size.x);
  light->SetLightColor(FLinearColor(model->color));
}

void ALED::SetColor(FColor color) {
  light->SetLightColor(FLinearColor(color));
}