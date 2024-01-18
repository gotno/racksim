#include "ModuleComponents/VCVButton.h"

#include "osc3.h"
#include "VCVData/VCV.h"
#include "osc3GameModeBase.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "PhysicsEngine/BodySetup.h"

AVCVButton::AVCVButton() {
  MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
  SetRootComponent(MeshComponent);

  MeshComponent->SetGenerateOverlapEvents(true);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  MeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  MeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);
  MeshComponent->SetCollisionResponseToChannel(INTERACTOR_OBJECT, ECollisionResponse::ECR_Overlap);
  
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

void AVCVButton::BeginPlay() {
  Super::BeginPlay();

  UBodySetup* bodySetup = MeshComponent->GetBodySetup();
  if (bodySetup) bodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    MeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    MeshComponent->SetMaterial(1, FaceMaterialInstance);
  }

  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVButton::Tick(float DeltaTime) {
  for (int i = 0; i < model->svgPaths.Num(); i++) {
    if (!frames[i]) {
      frames[i] = gameMode->GetTexture(model->svgPaths[i]);
      if (frames[i] && model->value == i) {
        FaceMaterialInstance->SetTextureParameterValue(FName("texture"), frames[i]);
      }
    }
  }
}

void AVCVButton::init(VCVParam* vcv_param) {
  Super::init(vcv_param);

  // remove empty svg paths and init frames array to same size
  vcv_param->svgPaths.Remove(FString(""));
  frames.Init(nullptr, vcv_param->svgPaths.Num());

  spawnLights(MeshComponent);
  BaseMaterialInstance->SetVectorParameterValue(TEXT("color"), model->bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(TEXT("background_color"), model->bodyColor);
}

void AVCVButton::engage() {
  Super::engage();
  setValue(model->value == model->minValue ? model->maxValue : model->minValue);
  FaceMaterialInstance->SetTextureParameterValue(
    FName("texture"),
    model->value == model->minValue ? frames[0] : frames[frames.Num() - 1]
  );
}

void AVCVButton::release() {
  Super::release();
  if (model->momentary) {
    setValue(model->minValue);
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), frames[model->value]);
  }
}

void AVCVButton::Update(VCVParam& Param) {
  Super::Update(Param);

  FaceMaterialInstance->SetTextureParameterValue(
    FName("texture"),
    model->value == model->minValue ? frames[0] : frames[frames.Num() - 1]
  );
}