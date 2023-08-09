#include "VCVDisplay.h"

#include "osc3.h"
#include "VCV.h"

#include "UObject/ConstructorHelpers.h"

AVCVDisplay::AVCVDisplay() {
	PrimaryActorTick.bCanEverTick = true;

  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;

  BaseMeshComponent->SetGenerateOverlapEvents(true);
  BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BaseMeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  BaseMeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);
  
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

void AVCVDisplay::init(VCVDisplay* vcv_display) {
  model = vcv_display;
  SetActorScale3D(FVector(1.f, model->box.size.x, model->box.size.y));
}