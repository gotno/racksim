#include "ModuleComponents/VCVPort.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCVData/VCV.h"
#include "VCVModule.h"
#include "CableEnd.h"

#include "engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVPort::AVCVPort() {
	PrimaryActorTick.bCanEverTick = true;

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = StaticMeshComponent;

  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetCollisionObjectType(PORT_OBJECT);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_port_faced.unit_port_faced'"));
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

  // loading indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial>
    LoadingMaterialFinder(LoadingMaterialRef);
  if (LoadingMaterialFinder.Object)
    LoadingMaterialInterface = Cast<UMaterial>(LoadingMaterialFinder.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'"));
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }
}

void AVCVPort::BeginPlay() {
	Super::BeginPlay();
  
  Module = Cast<AVCVModule>(GetOwner());
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  if (LoadingMaterialInterface) {
    LoadingMaterialInstance = UMaterialInstanceDynamic::Create(LoadingMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, LoadingMaterialInstance);
  }

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
  }
  
  Tags.Add(TAG_INTERACTABLE_PORT);
}

void AVCVPort::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVPort::Init(VCVPort* vcv_port) {
  Model = vcv_port;

  Id = vcv_port->id;
  Type = vcv_port->type;

  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE, Model->box.size.x, Model->box.size.y));
  
  BaseMaterialInstance->SetVectorParameterValue(FName("color"), Model->bodyColor);
  FaceMaterialInstance->SetVectorParameterValue(FName("background_color"), Model->bodyColor);

  GameMode->RequestTexture(Model->svgPath, this, FName("SetTexture"));
}

void AVCVPort::SetTexture(FString Filepath, UTexture2D* inTexture) {
  if (!Texture && Filepath.Equals(Model->svgPath)) {
    Texture = inTexture;
    StaticMeshComponent->SetMaterial(1, FaceMaterialInstance);
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
  }
}

bool AVCVPort::CanConnect(PortType inType) {
  if (inType != PortType::Any && inType != Type) return false;
  if (Type == PortType::Input && HasConnections()) return false;
  return true;
}

bool AVCVPort::HasConnections() {
  return ConnectedCableEnds.Num() > 0;
}

ACableEnd* AVCVPort::GetTopCableEnd() {
  if (!HasConnections()) return nullptr;
  return ConnectedCableEnds[ConnectedCableEnds.Num() - 1];
}

void AVCVPort::Connect(ACableEnd* CableEnd) {
  checkf(
    Type == PortType::Output || !HasConnections(),
    TEXT("attempting to add more than one cable to an Input")
  );

  ConnectedCableEnds.Add(CableEnd);
}

void AVCVPort::Disconnect(ACableEnd* CableEnd) {
  checkf(
    HasConnections(),
    TEXT("attempting to remove a cable from an empty port")
  );

  ConnectedCableEnds.Remove(CableEnd);
}

void AVCVPort::TriggerCableUpdates() {
  for (ACableEnd* cableEnd : ConnectedCableEnds) {
    cableEnd->UpdatePosition();
  }
}

void AVCVPort::GetTooltipText(FString& Name, FString& Description) {
  Name = Model->name;
  if (Name.IsEmpty()) {
    Name = FString("#");
    Name.AppendInt(Model->id + 1);
  }
  Name.Append(Type == PortType::Input ? " input" : " output");
  Description = Model->description;
}