#include "VCVSlider.h"

#include "osc3.h"
#include "VCV.h"
#include "osc3GameModeBase.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVSlider::AVCVSlider() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
  
  // base mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBase(BaseMeshReference);
  if (MeshBase.Object) BaseMeshComponent->SetStaticMesh(MeshBase.Object);
  SetRootComponent(BaseMeshComponent);

  // base materials
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseFaceMaterial(BaseFaceMaterialReference);
  if (BaseFaceMaterial.Object) {
    BaseFaceMaterialInterface = Cast<UMaterial>(BaseFaceMaterial.Object);
  }

  HandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle Mesh"));
  HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  // handle mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshHandle(HandleMeshReference);
  if (MeshHandle.Object) HandleMeshComponent->SetStaticMesh(MeshHandle.Object);
  HandleMeshComponent->SetupAttachment(GetRootComponent());

  HandleMeshComponent->SetGenerateOverlapEvents(true);
  HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  HandleMeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  HandleMeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);

  // handle materials
  static ConstructorHelpers::FObjectFinder<UMaterial> HandleMaterial(HandleMaterialReference);
  if (HandleMaterial.Object) {
    HandleMaterialInterface = Cast<UMaterial>(HandleMaterial.Object);
  }
  static ConstructorHelpers::FObjectFinder<UMaterial> HandleFaceMaterial(HandleFaceMaterialReference);
  if (HandleFaceMaterial.Object) {
    HandleFaceMaterialInterface = Cast<UMaterial>(HandleFaceMaterial.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVSlider::BeginPlay() {
	Super::BeginPlay();
	
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (BaseFaceMaterialInterface) {
    BaseFaceMaterialInstance = UMaterialInstanceDynamic::Create(BaseFaceMaterialInterface, this);
    BaseMeshComponent->SetMaterial(1, BaseFaceMaterialInstance);
  }

  if (HandleMaterialInterface) {
    HandleMaterialInstance = UMaterialInstanceDynamic::Create(HandleMaterialInterface, this);
    HandleMeshComponent->SetMaterial(0, HandleMaterialInstance);
    HandleMaterialInstance->SetVectorParameterValue(FName("Color"), FColor::Black);
  }
  if (HandleFaceMaterialInterface) {
    HandleFaceMaterialInstance = UMaterialInstanceDynamic::Create(HandleFaceMaterialInterface, this);
    HandleMeshComponent->SetMaterial(1, HandleFaceMaterialInstance);
  }
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVSlider::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (!baseTexture) {
    baseTexture = gameMode->GetTexture(model->svgPaths[0]);
    if (baseTexture) BaseFaceMaterialInstance->SetTextureParameterValue(FName("texture"), baseTexture);
  }

  if (!handleTexture) {
    handleTexture = gameMode->GetTexture(model->svgPaths[1]);
    if (handleTexture) HandleFaceMaterialInstance->SetTextureParameterValue(FName("texture"), handleTexture);
  }
}

void AVCVSlider::init(VCVParam* vcv_param) {
	Super::init(vcv_param);

  FVector minHandlePosition = GetActorLocation() + FVector(0, model->minHandlePos.x, model->minHandlePos.y);
  FVector maxHandlePosition = GetActorLocation() + FVector(0, model->maxHandlePos.x, model->maxHandlePos.y);
  
  HandleMeshComponent->SetWorldScale3D(FVector(1, model->handleBox.size.x, model->handleBox.size.y));
  spawnLights(HandleMeshComponent);

  if (model->horizontal) {
    direction = HandleMeshComponent->GetRightVector();
    minMaxOffsetDelta = maxHandlePosition.Y - minHandlePosition.Y;
  } else {
    direction = HandleMeshComponent->GetUpVector();
    minMaxOffsetDelta = maxHandlePosition.Z - minHandlePosition.Z;
  }

  worldOffset = getOffsetFromValue();
  HandleMeshComponent->SetWorldLocation(minHandlePosition + direction * worldOffset);
  shadowOffset = worldOffset;
}

float AVCVSlider::getOffsetFromValue() {
  float valuePercent = (model->value - model->minValue) / (model->maxValue - model->minValue);
  return valuePercent * minMaxOffsetDelta;
}

float AVCVSlider::getValueFromOffset() {
  float offsetPercent = shadowOffset / minMaxOffsetDelta;
  float value = model->minValue + offsetPercent * (model->maxValue - model->minValue);
  if (model->snap) value = round(value);
  return value;
}

void AVCVSlider::engage() {
  Super::engage();

  direction =
    model->horizontal
      ? HandleMeshComponent->GetRightVector()
      : HandleMeshComponent->GetUpVector();

  lastValue = model->value;
}

void AVCVSlider::alter(float amount) {
  Super::alter(amount);
  if (amount == lastAlterAmount) return;

  float change = (amount - lastAlterAmount) * alterRatio;
  shadowOffset = FMath::Clamp(shadowOffset + change, 0.f, minMaxOffsetDelta);

  setValue(getValueFromOffset());
  FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - direction * worldOffset;
  worldOffset = model->snap ? getOffsetFromValue() : shadowOffset;
  HandleMeshComponent->SetWorldLocation(zeroPosition + direction * worldOffset);

  lastAlterAmount = amount;
}

void AVCVSlider::release() {
  Super::release();
  
  // treat snapping sliders like a switch and increment
  // if they haven't been dragged enough to change value
  if (model->snap && model->value == lastValue) {
    float newValue = model->value + 1;
    if (newValue > model->maxValue) newValue = 0;
    setValue(newValue);
    FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - direction * worldOffset;
    worldOffset = getOffsetFromValue();
    HandleMeshComponent->SetWorldLocation(zeroPosition + direction * worldOffset);
  }

  lastValue = model->value;
  lastAlterAmount = 0.f;
  shadowOffset = worldOffset;
}