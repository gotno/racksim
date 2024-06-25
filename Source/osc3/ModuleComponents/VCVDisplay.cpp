#include "ModuleComponents/VCVDisplay.h"

#include "osc3.h"
#include "VCVData/VCV.h"

#include "UObject/ConstructorHelpers.h"

AVCVDisplay::AVCVDisplay() {
	PrimaryActorTick.bCanEverTick = true;

  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;

  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_lcd.unit_lcd'"));
  
  if (MeshBody.Object) BaseMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Script/Engine.Material'/Game/materials/display_blank.display_blank'"));
  
  if (Material.Object) {
    MaterialInterface = Cast<UMaterial>(Material.Object);
  }

  SetActorEnableCollision(true);
}

void AVCVDisplay::BeginPlay() {
	Super::BeginPlay();

  if (MaterialInterface) {
    MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, MaterialInstance);
  }

}

void AVCVDisplay::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVDisplay::Init(VCVDisplay* vcv_display) {
  Model = vcv_display;
  SetActorScale3D(FVector(1.f, Model->box.size.x, Model->box.size.y));
}