#include "VCVSwitch.h"

#include "osc3.h"
#include "VCV.h"
#include "osc3GameModeBase.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVSwitch::AVCVSwitch() {
  MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
  SetRootComponent(MeshComponent);

  MeshComponent->SetGenerateOverlapEvents(true);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  MeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  MeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(MeshReference);
  if (Mesh.Object) MeshComponent->SetStaticMesh(Mesh.Object);

  // base material
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

  // face material
  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(FaceMaterialReference);
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }

  SetActorEnableCollision(true);
}

void AVCVSwitch::BeginPlay() {
	Super::BeginPlay();

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    MeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FColor(181, 170, 169, 255));
  }
  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    MeshComponent->SetMaterial(1, FaceMaterialInstance);
  }

  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVSwitch::Tick(float DeltaTime) {
  for (int i = 0; i < model->svgPaths.Num(); i++) {
    if (!frames[i]) {
      frames[i] = gameMode->GetTexture(model->svgPaths[i]);
      if (getFrameFromValue() == i && frames[i]) setFrame();
    }
  }
}

void AVCVSwitch::init(VCVParam* vcv_param) {
	Super::init(vcv_param);
  
  // remove empty svg paths and init frames array to same size
  vcv_param->svgPaths.Remove(FString(""));
  frames.Init(nullptr, vcv_param->svgPaths.Num());
  
  SetActorScale3D(FVector(1, model->box.size.x, model->box.size.y));
}

void AVCVSwitch::engage() {
  Super::engage();
  float newValue = model->value + 1;
  if (newValue > model->maxValue) newValue = model->minValue;
  setValue(newValue);
  setFrame();
}

// some values don't align with frame index
// (looking at you, instruo)
// get the distance from min value instead
int AVCVSwitch::getFrameFromValue() {
  return model->value - model->minValue;
}

void AVCVSwitch::setFrame() {
  int frame = getFrameFromValue();
  if (frame >= 0 && frame < frames.Num())
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), frames[frame]);
}