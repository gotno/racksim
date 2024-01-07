#include "VCVPort.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVOverrides.h"
#include "VCVModule.h"

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

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'"));
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
  
  Tags.Add(TAG_INTERACTABLE_PORT);
}

void AVCVPort::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (texture) return;
  
  texture = gameMode->GetTexture(model->svgPath);
  if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
}

void AVCVPort::init(VCVPort* vcv_port) {
  model = vcv_port;

  Type = vcv_port.type;

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE, model->box.size.x, model->box.size.y));
  
  BaseMaterialInstance->SetVectorParameterValue(FName("color"), model->bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), model->bodyColor);
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

bool AVCVPort::CanConnect(PortType inType) {
  if (inType != Type) return false;
  if (Type == PortType::Input && Cables.Num() > 0) return false;
  return true;
}

void AVCVPort::AddCable(AVCVCable* Cable) {
  checkf(
    Type == PortType::Output || Cables.Num() == 0,
    TEXT("attempting to add more than one cable to an Input")
  );

  Cables.Add(Cable);
}

void AVCVPort::RemoveCable(AVCVCable* Cable) {
  checkf(
    Cables.Num() > 0,
    TEXT("attempting to remove a cable from an empty port")
  );

  Cables.Remove(Cable);
}

PortIdentity AVCVPort::getIdentity() {
  return model->getIdentity();
}

void AVCVPort::GetTooltipText(FString& Name, FString& Description) {
  Name = model->name;
  if (Name.IsEmpty()) {
    Name = FString("#");
    Name.AppendInt(model->id + 1);
  }
  Name.Append(Type == PortType::Input ? " input" : " output");
  Description = model->description;
}
