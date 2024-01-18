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

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVButton::Tick(float DeltaTime) {
  for (int i = 0; i < Model->svgPaths.Num(); i++) {
    if (!Frames[i]) {
      Frames[i] = GameMode->GetTexture(Model->svgPaths[i]);
      if (Frames[i] && Model->value == i) {
        FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Frames[i]);
      }
    }
  }
}

void AVCVButton::Init(VCVParam* vcv_param) {
  Super::Init(vcv_param);

  // remove empty svg paths and init frames array to same size
  vcv_param->svgPaths.Remove(FString(""));
  Frames.Init(nullptr, vcv_param->svgPaths.Num());

  SpawnLights(MeshComponent);
  BaseMaterialInstance->SetVectorParameterValue(TEXT("color"), Model->bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(TEXT("background_color"), Model->bodyColor);
}

void AVCVButton::Engage() {
  Super::Engage();
  SetValue(Model->value == Model->minValue ? Model->maxValue : Model->minValue);
  FaceMaterialInstance->SetTextureParameterValue(
    FName("texture"),
    Model->value == Model->minValue ? Frames[0] : Frames[Frames.Num() - 1]
  );
}

void AVCVButton::Release() {
  Super::Release();
  if (Model->momentary) {
    SetValue(Model->minValue);
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Frames[Model->value]);
  }
}

void AVCVButton::Update(VCVParam& vcv_param) {
  Super::Update(vcv_param);

  FaceMaterialInstance->SetTextureParameterValue(
    FName("texture"),
    Model->value == Model->minValue ? Frames[0] : Frames[Frames.Num() - 1]
  );
}