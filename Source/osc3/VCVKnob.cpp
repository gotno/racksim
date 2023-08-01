#include "VCVKnob.h"

#include "osc3GameModeBase.h"
#include "VCV.h"

#include "engine/Texture2D.h"

#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVKnob::AVCVKnob() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;
  // BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(MeshReference);
  
  if (MeshBody.Object) BaseMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(FaceMaterialReference);
  
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }
}

void AVCVKnob::BeginPlay() {
	Super::BeginPlay();
	
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    BaseMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}
  
void AVCVKnob::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  if (!textureBackground && !model->backgroundSvgPath.IsEmpty()) {
    textureBackground = gameMode->GetTexture(model->backgroundSvgPath);
    if (textureBackground) FaceMaterialInstance->SetTextureParameterValue(FName("texture_bg"), textureBackground);
  }
  if (!texture && !model->svgPath.IsEmpty()) {
    texture = gameMode->GetTexture(model->svgPath);
    if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
  }
  if (!textureForeground && !model->foregroundSvgPath.IsEmpty()) {
    textureForeground = gameMode->GetTexture(model->foregroundSvgPath);
    if (textureForeground) FaceMaterialInstance->SetTextureParameterValue(FName("texture_fg"), textureForeground);
  }
}

void AVCVKnob::init(VCVParam* vcv_param) {
	Super::init(vcv_param);
  updateRotation(getRotationFromValue());
}

void AVCVKnob::updateRotation(FRotator newRotation) {
  FaceMaterialInstance->SetScalarParameterValue(FName("rotation"), -newRotation.Roll);
  SetActorRotation(newRotation);
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
  updateRotation(model->snap ? getRotationFromValue() : shadowRotation);

  lastAmount = amount;
}

void AVCVKnob::release() {
  Super::release();
  lastAmount = 0.f;
}