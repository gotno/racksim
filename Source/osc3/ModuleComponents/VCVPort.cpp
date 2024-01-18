#include "ModuleComponents/VCVPort.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCVData/VCV.h"
#include "VCVModule.h"
#include "VCVCable.h"

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
  
  Module = Cast<AVCVModule>(GetOwner());
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }
  
  Tags.Add(TAG_INTERACTABLE_PORT);
}

void AVCVPort::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (Texture) return;
  
  Texture = GameMode->GetTexture(model->svgPath);
  if (Texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
}

void AVCVPort::init(VCVPort* vcv_port) {
  model = vcv_port;

  Id = vcv_port->id;
  Type = vcv_port->type;

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE, model->box.size.x, model->box.size.y));
  
  BaseMaterialInstance->SetVectorParameterValue(FName("color"), model->bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), model->bodyColor);
}

bool AVCVPort::CanConnect(PortType inType) {
  if (inType != Type) return false;
  if (Type == PortType::Input && HasCables()) return false;
  return true;
}

bool AVCVPort::HasCables() {
  return Cables.Num() > 0;
}

AVCVCable* AVCVPort::GetTopCable() {
  if (!HasCables()) return nullptr;
  return Cables[Cables.Num() - 1];
}

void AVCVPort::AddCable(AVCVCable* Cable) {
  checkf(
    Type == PortType::Output || !HasCables(),
    TEXT("attempting to add more than one cable to an Input")
  );

  Cables.Add(Cable);
}

void AVCVPort::RemoveCable(AVCVCable* Cable) {
  checkf(
    HasCables(),
    TEXT("attempting to remove a cable from an empty port")
  );

  Cables.Remove(Cable);
}

void AVCVPort::TriggerCableUpdates() {
  for (AVCVCable* cable : Cables) {
    cable->UpdateEndPositions();
  }
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