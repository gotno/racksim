#include "VCVButton.h"

#include "VCV.h"

AVCVButton::AVCVButton() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;
  // BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(MeshReference);
  
  if (MeshBody.Object) BaseMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> Material(MaterialReference);
  
  if (Material.Object) {
    MaterialInterface = Cast<UMaterial>(Material.Object);
  }
}

void AVCVButton::BeginPlay() {
  Super::BeginPlay();

  if (MaterialInterface) {
    MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, MaterialInstance);
    MaterialInstance->SetVectorParameterValue(TEXT("Color"), FColor(243, 233, 159, 255));
  }
}

void AVCVButton::setModel(VCVParam* vcv_param) {
  Super::setModel(vcv_param);
  // spawnLights(HandleMeshComponent);
}

void AVCVButton::engage() {
  Super::engage();
  setValue(model->value == 0 ? 1 : 0);
}

void AVCVButton::release() {
  Super::release();
  if (model->momentary) {
    setValue(0);
  }
}