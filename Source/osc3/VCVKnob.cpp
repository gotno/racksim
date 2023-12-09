#include "VCVKnob.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVOverrides.h"

#include "engine/Texture2D.h"

#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AVCVKnob::AVCVKnob() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;
  
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
  if (!textureBackground && !model->svgPaths[0].IsEmpty()) {
    textureBackground = gameMode->GetTexture(model->svgPaths[0]);
    if (textureBackground) FaceMaterialInstance->SetTextureParameterValue(FName("texture_bg"), textureBackground);
  }
  if (!texture && !model->svgPaths[1].IsEmpty()) {
    texture = gameMode->GetTexture(model->svgPaths[1]);
    if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
  }
  if (!textureForeground && !model->svgPaths[2].IsEmpty()) {
    textureForeground = gameMode->GetTexture(model->svgPaths[2]);
    if (textureForeground) FaceMaterialInstance->SetTextureParameterValue(FName("texture_fg"), textureForeground);
  }
}

void AVCVKnob::init(VCVParam* vcv_param) {
	Super::init(vcv_param);
  updateRotation(getRotationFromValue());

  VCVOverrides overrides;
  FVector scaleMultiplier = overrides.getScaleMultiplier(getModuleBrand(), model->svgPaths[1]);
  SetActorScale3D(GetActorScale3D() * scaleMultiplier);

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), model->bodyColor);
}

void AVCVKnob::updateRotation(FRotator newRotation) {
  FaceMaterialInstance->SetScalarParameterValue(FName("rotation"), -newRotation.Roll);
  BaseMeshComponent->SetRelativeRotation(newRotation);
}

FRotator AVCVKnob::getRotationFromValue() {
  float valuePercent = (model->value - model->minValue) / (model->maxValue - model->minValue);
  float valueAngle = model->minAngle + valuePercent * (model->maxAngle - model->minAngle);

  FRotator knobRotation = BaseMeshComponent->GetRelativeRotation();
  return FRotator(knobRotation.Pitch, knobRotation.Yaw, valueAngle);
}

float AVCVKnob::getValueFromRotation() {
  float roll = shadowRotation.Roll;
  float rotationPercent = (roll - model->minAngle) / (model->maxAngle - model->minAngle);
  float value = model->minValue + ((model->maxValue - model->minValue) * rotationPercent);

  if (model->snap) value = round(value);
  return value;
}

void AVCVKnob::engage(float ControllerRoll) {
  Super::engage(ControllerRoll);
  shadowRotation = BaseMeshComponent->GetRelativeRotation();
  LastControllerRoll = ControllerRoll;
}

void AVCVKnob::alter(float ControllerRoll) {
  Super::alter(ControllerRoll);

  float deltaRoll = ControllerRoll - LastControllerRoll;
  // TODO: magic number?
  // limiting max roll per tick fixes jump from min to max or max to min
  if (FMath::Abs(deltaRoll) > 30) {
    LastControllerRoll = ControllerRoll;
    return;
  }
  deltaRoll *= alterRatio;

  // TODO: user override of smoothing constant
  deltaRoll =
    FMath::WeightedMovingAverage(deltaRoll, LastDeltaRoll, 0.2f);
  LastDeltaRoll = deltaRoll;

  FRotator knobRotation = BaseMeshComponent->GetRelativeRotation();
  shadowRotation.Pitch = knobRotation.Pitch;
  shadowRotation.Yaw = knobRotation.Yaw;
  shadowRotation.Roll =
    FMath::Clamp(shadowRotation.Roll + deltaRoll, model->minAngle, model->maxAngle);


  setValue(getValueFromRotation());
  updateRotation(model->snap ? getRotationFromValue() : shadowRotation);

  LastControllerRoll = ControllerRoll;
}

void AVCVKnob::release() {
  Super::release();
}