#include "VCVLight.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "VCV.h"

#include "Math/UnrealMathUtility.h"

AVCVLight::AVCVLight() {
	PrimaryActorTick.bCanEverTick = true;

  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;
  BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshCircle(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_led_round.unit_led_round'"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRectangle(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_led_rectangle.unit_led_rectangle'"));
  
  if (MeshCircle.Object) circleMesh = Cast<UStaticMesh>(MeshCircle.Object);
  if (MeshRectangle.Object) rectangleMesh = Cast<UStaticMesh>(MeshRectangle.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Script/Engine.Material'/Game/materials/led.led'"));
  
  if (Material.Object) {
    MaterialInterface = Cast<UMaterial>(Material.Object);
  }
}

void AVCVLight::BeginPlay() {
	Super::BeginPlay();
  
  if (MaterialInterface) {
    MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, MaterialInstance);
  }
}

void AVCVLight::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVLight::init(VCVLight* vcv_light) {
  model = vcv_light;
  SetActorScale3D(FVector(1.f, model->box.size.x, model->box.size.y));

  SetColor(model->bgColor);
  SetEmissiveColor(model->color);
  SetEmissiveIntensity(model->color.A);

  if (model->shape == LightShape::Round) {
    BaseMeshComponent->SetStaticMesh(circleMesh);
  } else {
    BaseMeshComponent->SetStaticMesh(rectangleMesh);
  }
}

void AVCVLight::SetColor(FLinearColor color) {
  MaterialInstance->SetVectorParameterValue(TEXT("Color"), color);
}

void AVCVLight::SetEmissiveColor(FLinearColor color) {
  if (color.A == 0.f) color = FLinearColor(0.f, 0.f, 0.f, 0.f);
  MaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), color);
}

void AVCVLight::SetEmissiveIntensity(float intensity) {
  MaterialInstance->SetScalarParameterValue(TEXT("EmissiveIntensity"), intensity * 2);
  // MaterialInstance->SetScalarParameterValue(TEXT("Transparency"), intensity);
}