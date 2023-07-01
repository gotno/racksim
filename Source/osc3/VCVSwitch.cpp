#include "VCVSwitch.h"

#include "VCV.h"

AVCVSwitch::AVCVSwitch() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
  
  // base mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBase(BaseMeshReference);
  if (MeshBase.Object) BaseMeshComponent->SetStaticMesh(MeshBase.Object);
  SetRootComponent(BaseMeshComponent);

  // base material
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

  HandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle Mesh"));
  
  // handle mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshHandle(HandleMeshReference);
  if (MeshHandle.Object) HandleMeshComponent->SetStaticMesh(MeshHandle.Object);
  HandleMeshComponent->SetupAttachment(GetRootComponent());

  // handle material
  static ConstructorHelpers::FObjectFinder<UMaterial> HandleMaterial(HandleMaterialReference);
  if (HandleMaterial.Object) {
    HandleMaterialInterface = Cast<UMaterial>(HandleMaterial.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVSwitch::BeginPlay() {
	Super::BeginPlay();
	
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FColor(181, 170, 169, 255));
  }
  if (HandleMaterialInterface) {
    HandleMaterialInstance = UMaterialInstanceDynamic::Create(HandleMaterialInterface, this);
    HandleMeshComponent->SetMaterial(0, HandleMaterialInstance);
    HandleMaterialInstance->SetVectorParameterValue(TEXT("Color"), FColor(255, 109, 96, 255));
  }
}

void AVCVSwitch::init(VCVParam* vcv_param) {
	Super::init(vcv_param);
  
  // TODO: frame count is just maxValue + 1
  // but also this isn't the way every switch works so maybe don't TODO
  FVector zeroPosition;
  if (model->horizontal) {
    handleSize = FVector2D(model->box.size.x / model->frameCount, model->box.size.y);
    direction = HandleMeshComponent->GetRightVector();
    zeroPosition = GetActorLocation() - FVector(0, model->box.size.x * 0.5f - handleSize.X * 0.5f, 0);
    HandleMeshComponent->SetWorldScale3D(FVector(1, handleSize.X, handleSize.Y));
  } else {
    handleSize = FVector2D(model->box.size.x, model->box.size.y / model->frameCount);
    direction = HandleMeshComponent->GetUpVector();
    zeroPosition = GetActorLocation() - FVector(0, 0, model->box.size.y * 0.5f - handleSize.Y * 0.5f);
    HandleMeshComponent->SetWorldScale3D(FVector(1, handleSize.X, handleSize.Y));
  }
  worldOffset = getOffsetFromValue();
  HandleMeshComponent->SetWorldLocation(zeroPosition + direction * worldOffset);
}

float AVCVSwitch::getOffsetFromValue() {
  if (model->horizontal) {
    return model->value * handleSize.X;
  }
  return model->value * handleSize.Y;
}

void AVCVSwitch::engage() {
  Super::engage();
  float newValue = model->value + 1;
  if (newValue > model->maxValue) newValue = 0;
  FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - direction * worldOffset;
  setValue(newValue);
  worldOffset = getOffsetFromValue();
  HandleMeshComponent->SetWorldLocation(zeroPosition + direction * worldOffset);
}