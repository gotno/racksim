#include "VCVParam.h"

#include "VCV.h"
#include "VCVModule.h"
#include "VCVLight.h"

AVCVParam::AVCVParam() {
	PrimaryActorTick.bCanEverTick = true;
}

void AVCVParam::BeginPlay() {
	Super::BeginPlay();
}

void AVCVParam::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (engaged) {
    // UE_LOG(LogTemp, Warning, TEXT("engaged %s"), *GetName());
  }
}

void AVCVParam::setModel(VCVParam* vcv_param) {
  model = vcv_param; 
  owner = Cast<AVCVModule>(GetOwner());
  SetActorScale3D(FVector(1, model->box.size.x, model->box.size.y));
}

void AVCVParam::SpawnLights(USceneComponent* attachTo) {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  for (TPair<int32, VCVLight>& light_kvp : model->Lights) {
    VCVLight& light = light_kvp.Value;

    FVector lightLocation = GetActorLocation() + lightOffset;
    if (model->type == ParamType::Switch || model->type == ParamType::Slider) {
      lightLocation += model->handleBox.location();
    }

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      lightLocation,
      FRotator(0, 0, 0),
      spawnParams
    );
    a_light->AttachToComponent(attachTo, FAttachmentTransformRules::KeepWorldTransform);
    a_light->SetModel(&light);
    owner->registerParamLight(light.id, a_light);
  }
}

void AVCVParam::setValue(float newValue) {
  if (newValue == model->value) return;

  model->value = newValue;
  owner->paramUpdated(model->id, model->value);
}

void AVCVParam::engage() {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  engaged = true;
}

void AVCVParam::alter(float amount) {
  // UE_LOG(LogTemp, Warning, TEXT("param alter %f, ratio'd: %f"), amount, amount * alterRatio);
}

void AVCVParam::release() {
  // UE_LOG(LogTemp, Warning, TEXT("param release"));
  engaged = false;
}