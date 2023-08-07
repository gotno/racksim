#include "VCVLight.h"

#include "osc3.h"
#include "VCV.h"
#include "VCVKnob.h"
#include "VCVParam.h"
#include "VCVButton.h"
#include "VCVSlider.h"
#include "VCVSwitch.h"

#include "Engine.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AVCVLight::AVCVLight() {
	PrimaryActorTick.bCanEverTick = true;

  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;

  BaseMeshComponent->SetGenerateOverlapEvents(true);
  BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BaseMeshComponent->SetCollisionObjectType(LIGHT_OBJECT);
  BaseMeshComponent->SetCollisionResponseToChannel(PARAM_OBJECT, ECollisionResponse::ECR_Overlap);
  BaseMeshComponent->SetCollisionResponseToChannel(PARAM_TRACE, ECollisionResponse::ECR_Ignore);
  
  BaseMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AVCVLight::onBeginOverlap);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshCircle(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_led_round_faced.unit_led_round_faced'"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRectangle(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_led_rect_faced.unit_led_rect_faced'"));
  
  if (MeshCircle.Object) circleMesh = Cast<UStaticMesh>(MeshCircle.Object);
  if (MeshRectangle.Object) rectangleMesh = Cast<UStaticMesh>(MeshRectangle.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Script/Engine.Material'/Game/materials/led.led'"));
  
  if (Material.Object) {
    MaterialInterface = Cast<UMaterial>(Material.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVLight::BeginPlay() {
	Super::BeginPlay();
  
  if (MaterialInterface) {
    MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, MaterialInstance);
    BaseMeshComponent->SetMaterial(1, MaterialInstance);
  }
}

void AVCVLight::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVLight::onBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (Cast<AVCVSwitch>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with switch %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
  } else if (Cast<AVCVButton>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with button %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 0.2f);
  } else if (Cast<AVCVSlider>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with slider %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 0.2f);
  } else if (Cast<AVCVKnob>(OtherActor)) {
    // TODO: overlap is a little eager? shouldn't be overlapping with neighboring knobs.
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with knob %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    // SetActorLocation(GetActorLocation() - GetActorForwardVector() * 1.f);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("%s:%s overlapped with something else: %s"), *GetOwner()->GetActorNameOrLabel(), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
  }
}

void AVCVLight::init(VCVLight* vcv_light) {
  model = vcv_light;

  SetColor(model->bgColor);
  SetEmissiveColor(model->color);
  SetEmissiveIntensity(model->color.A);
  
  if (model->shape == LightShape::Round) {
    BaseMeshComponent->SetStaticMesh(circleMesh);
  } else {
    BaseMeshComponent->SetStaticMesh(rectangleMesh);
  }
  
  SetActorScale3D(FVector(0.01f, model->box.size.x, model->box.size.y));
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
  if (model->transparent) {
    MaterialInstance->SetScalarParameterValue(TEXT("Transparency"), intensity);
  }
}