#include "VCVLight.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVKnob.h"
#include "VCVParam.h"
#include "VCVButton.h"
#include "VCVSlider.h"
#include "VCVSwitch.h"

#include "Engine.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
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
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshCircle(CircleMeshReference);
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRectangle(RectangleMeshReference);
  
  if (MeshCircle.Object) circleMesh = Cast<UStaticMesh>(MeshCircle.Object);
  if (MeshRectangle.Object) rectangleMesh = Cast<UStaticMesh>(MeshRectangle.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(FaceMaterialReference);
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVLight::BeginPlay() {
	Super::BeginPlay();
  
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }

  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    BaseMeshComponent->SetMaterial(1, FaceMaterialInstance);
  }
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVLight::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  if (!texture && !model->svgPath.IsEmpty()) {
    texture = gameMode->GetTexture(model->svgPath);
    if (texture) FaceMaterialInstance->SetTextureParameterValue(FName("texture"), texture);
  }
}

void AVCVLight::onBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (Cast<AVCVSwitch>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with switch %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 0.01f);
  } else if (Cast<AVCVButton>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with button %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 0.2f);
  } else if (Cast<AVCVSlider>(OtherActor)) {
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with slider %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    SetActorLocation(GetActorLocation() - GetActorForwardVector() * 0.2f);
  // } else if (Cast<AVCVKnob>(OtherActor)) {
    // TODO: overlap is a little eager? shouldn't be overlapping with neighboring knobs.
    // UE_LOG(LogTemp, Warning, TEXT("%s overlapped with knob %s"), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
    // SetActorLocation(GetActorLocation() - GetActorForwardVector() * 1.f);
  } else {
    // UE_LOG(LogTemp, Warning, TEXT("%s:%s overlapped with something else: %s"), *GetOwner()->GetActorNameOrLabel(), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());
  }
}

void AVCVLight::init(VCVLight* vcv_light) {
  model = vcv_light;

  SetColor(model->bgColor);
  SetEmissiveColor(model->color);
  SetEmissiveIntensity(model->color.A);

  SetHidden(!model->visible);
  SetActorEnableCollision(model->visible);
  
  if (model->shape == LightShape::Round) {
    BaseMeshComponent->SetStaticMesh(circleMesh);
  } else {
    BaseMeshComponent->SetStaticMesh(rectangleMesh);
  }
  
  SetActorScale3D(FVector(0.01f, model->box.size.x, model->box.size.y));
}

void AVCVLight::SetColor(FLinearColor color) {
  BaseMaterialInstance->SetVectorParameterValue(TEXT("color"), color);
  FaceMaterialInstance->SetVectorParameterValue(TEXT("color"), color);
}

void AVCVLight::SetEmissiveColor(FLinearColor color) {
  if (color.A == 0.f) color = FLinearColor(0.f, 0.f, 0.f, 0.f);
  BaseMaterialInstance->SetVectorParameterValue(TEXT("emissive_color"), color);
  FaceMaterialInstance->SetVectorParameterValue(TEXT("emissive_color"), color);
}

void AVCVLight::SetEmissiveIntensity(float intensity) {
  BaseMaterialInstance->SetScalarParameterValue(TEXT("emissive_intensity"), intensity);
  FaceMaterialInstance->SetScalarParameterValue(TEXT("emissive_intensity"), intensity);
  BaseMaterialInstance->SetScalarParameterValue(TEXT("transparency"), intensity);
  FaceMaterialInstance->SetScalarParameterValue(TEXT("transparency"), intensity);
}