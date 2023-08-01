#include "VCVPort.h"

#include "osc3GameModeBase.h"
#include "VCV.h"

#include "engine/Texture2D.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVPort::AVCVPort() {
	PrimaryActorTick.bCanEverTick = true;


  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = StaticMeshComponent;
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_port_faced.unit_port_faced'"));
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face.texture_face'"));
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }
}

void AVCVPort::BeginPlay() {
	Super::BeginPlay();

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVPort::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (texture) return;
  
  texture = gameMode->GetTexture(model->svgPath);
  if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
}

void AVCVPort::init(VCVPort* vcv_port) {
  model = vcv_port;
  StaticMeshComponent->SetWorldScale3D(FVector(1, model->box.size.x, model->box.size.y));
}

void AVCVPort::addCableId(int64_t cableId) {
  cableIds.Push(cableId);
}

void AVCVPort::removeCableId(int64_t cableId) {
  cableIds.Remove(cableId);
}

bool AVCVPort::getCableId(int64_t& cableId) {
  if (cableIds.Num() == 0) return false;
  cableId = cableIds.Pop();
  return true;
}

bool AVCVPort::canConnect(PortType type) {
  if (type != model->type) return false;
  if (model->type == PortType::Input && hasCables()) return false;
  return true;
}

bool AVCVPort::hasCables() {
  return cableIds.Num() > 0;
}

PortIdentity AVCVPort::getIdentity() {
  return model->getIdentity();
}