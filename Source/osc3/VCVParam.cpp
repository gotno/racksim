#include "VCVParam.h"

#include "osc3.h"
#include "VCV.h"
#include "VCVModule.h"
#include "VCVLight.h"

#include "Misc/ScopeRWLock.h"

AVCVParam::AVCVParam() {
	PrimaryActorTick.bCanEverTick = true;
  
  Tags.Add(TAG_INTERACTABLE_PARAM);
}

void AVCVParam::BeginPlay() {
	Super::BeginPlay();
}

void AVCVParam::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVParam::init(VCVParam* vcv_param) {
  model = vcv_param; 
  owner = Cast<AVCVModule>(GetOwner());
  SetHidden(!model->visible);
  SetActorEnableCollision(model->visible);
  SetActorScale3D(FVector(RENDER_SCALE, model->box.size.x, model->box.size.y));
  // UE_LOG(LogTemp, Warning, TEXT("%s: %s%s"), *(model->name), *(model->displayValue), *(model->unit));
}

FString AVCVParam::getModuleBrand() {
  return owner->ModuleBrand;
}

void AVCVParam::GetTooltipText(FString& Label, FString& DisplayValue) {
  FScopeLock Lock(&DataGuard);
  Label = model->name;
  DisplayValue = model->displayValue;
}

void AVCVParam::UpdateDisplayValue(const FString& DisplayValue) {
  FScopeLock Lock(&DataGuard);
  model->displayValue = DisplayValue;
}

void AVCVParam::spawnLights(USceneComponent* attachTo) {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  for (TPair<int32, VCVLight>& light_kvp : model->Lights) {
    VCVLight& light = light_kvp.Value;

    FVector lightLocation = attachTo->GetComponentLocation();

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      lightLocation,
      FRotator(0, 0, 0),
      spawnParams
    );
    a_light->AttachToComponent(attachTo, FAttachmentTransformRules::KeepWorldTransform);
    a_light->init(&light);
    owner->registerParamLight(light.id, a_light);
  }
}

void AVCVParam::setValue(float newValue) {
  float roundedValue = FMath::RoundHalfToEven(newValue * 1000) / 1000;
  if (roundedValue == model->value) return;

  FScopeLock Lock(&DataGuard);
  model->value = roundedValue;
  owner->paramUpdated(model->id, model->value);
}

void AVCVParam::engage() {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  engaged = true;
}

void AVCVParam::engage(float _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  engaged = true;
}

void AVCVParam::engage(FVector _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  engaged = true;
}

void AVCVParam::alter(float amount) {
  // UE_LOG(LogTemp, Warning, TEXT("param alter %f, ratio'd: %f"), amount, amount * alterRatio);
}

void AVCVParam::alter(FVector _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param alter %s, ratio'd: %s"), *_value.ToCompactString(), *(_value * alterRatio).ToCompactString());
}

void AVCVParam::release() {
  // UE_LOG(LogTemp, Warning, TEXT("param release"));
  engaged = false;
}

void AVCVParam::resetValue() {
  // UE_LOG(LogTemp, Warning, TEXT("param reset"));
  if (model) setValue(model->defaultValue);
}