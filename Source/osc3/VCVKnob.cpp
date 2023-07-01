#include "VCVKnob.h"

#include "VCV.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AVCVKnob::AVCVKnob() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;
  // BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(MeshReference);
  
  if (MeshBody.Object) BaseMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  static ConstructorHelpers::FObjectFinder<UMaterial> MarkerMaterial(MarkerMaterialReference);
  
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  if (MarkerMaterial.Object) {
    MarkerMaterialInterface = Cast<UMaterial>(MarkerMaterial.Object);
  }
}

void AVCVKnob::BeginPlay() {
	Super::BeginPlay();
	
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (MarkerMaterialInterface) {
    MarkerMaterialInstance = UMaterialInstanceDynamic::Create(MarkerMaterialInterface, this);
    BaseMeshComponent->SetMaterial(1, MarkerMaterialInstance);
  }
}

void AVCVKnob::init(VCVParam* vcv_param) {
	Super::init(vcv_param);
  SetActorRotation(getRotationFromValue());
}

FRotator AVCVKnob::getRotationFromValue() {
  float valuePercent = (model->value - model->minValue) / (model->maxValue - model->minValue);
  float valueAngle = model->minAngle + valuePercent * (model->maxAngle - model->minAngle);

  FRotator actorRotation = GetActorRotation();
  return FRotator(actorRotation.Pitch, actorRotation.Yaw, valueAngle);
}

float AVCVKnob::getValueFromRotation() {
  float roll = shadowRotation.Roll;
  float rotationPercent = (roll - model->minAngle) / (model->maxAngle - model->minAngle);
  float value = model->minValue + ((model->maxValue - model->minValue) * rotationPercent);

  if (model->snap) value = round(value);
  return value;
}

void AVCVKnob::engage() {
  Super::engage();
  shadowRotation = GetActorRotation();
}

void AVCVKnob::alter(float amount) {
  Super::alter(amount);
  if (amount == lastAmount) return;

  float change = (amount - lastAmount) * alterRatio;
  shadowRotation.Roll =
    FMath::Clamp(shadowRotation.Roll + change, model->minAngle, model->maxAngle);

  setValue(getValueFromRotation());
  SetActorRotation(model->snap ? getRotationFromValue() : shadowRotation);

  lastAmount = amount;
}

void AVCVKnob::release() {
  Super::release();
  lastAmount = 0.f;
}