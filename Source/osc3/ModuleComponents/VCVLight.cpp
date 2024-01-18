#include "ModuleComponents/VCVLight.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCVData/VCV.h"
#include "VCVModule.h"

#include "Engine.h"
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
  BaseMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  BaseMeshComponent->SetCollisionResponseToChannel(PARAM_OBJECT, ECollisionResponse::ECR_Overlap);
  BaseMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AVCVLight::HandleBeginOverlap);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshCircle(CircleMeshReference);
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRectangle(RectangleMeshReference);
  
  if (MeshCircle.Object) circleMesh = Cast<UStaticMesh>(MeshCircle.Object);
  if (MeshRectangle.Object) rectangleMesh = Cast<UStaticMesh>(MeshRectangle.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVLight::BeginPlay() {
	Super::BeginPlay();
  
  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVLight::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVLight::Init(VCVLight* vcv_light) {
  Model = vcv_light;
  
  if (Model->shape == LightShape::Round) {
    BaseMeshComponent->SetStaticMesh(circleMesh);
  } else {
    BaseMeshComponent->SetStaticMesh(rectangleMesh);
  }

  SetColor(Model->bgColor);
  SetEmissiveColor(Model->color);
  SetEmissiveIntensity(Model->color.A);

  SetHidden(!Model->visible);
  SetActorEnableCollision(Model->visible);
  
  SetActorScale3D(FVector(1.f, Model->box.size.x, Model->box.size.y));
}

void AVCVLight::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (bHandledOverlap) return;

  AVCVModule* ownerModule = Cast<AVCVModule>(GetOwner());
  ownerModule = ownerModule ? ownerModule : Cast<AVCVModule>(GetOwner()->GetOwner());
  if (ownerModule != OtherActor->GetOwner()) return;

  FVector _, otherActorExtent;
  OtherActor->GetActorBounds(false, _, otherActorExtent);
  float delta = -otherActorExtent.X * 2;
  SetActorLocation(GetActorLocation() + GetActorForwardVector() * delta);

  bHandledOverlap = true;
}

void AVCVLight::SetColor(FLinearColor Color) {
  BaseMaterialInstance->SetVectorParameterValue(TEXT("color"), Color);
}

void AVCVLight::SetEmissiveColor(FLinearColor Color) {
  if (Color.A == 0.f) Color = FLinearColor(0.f, 0.f, 0.f, 0.f);
  BaseMaterialInstance->SetVectorParameterValue(TEXT("emissive_color"), Color);
}

void AVCVLight::SetEmissiveIntensity(float Intensity) {
  BaseMaterialInstance->SetScalarParameterValue(TEXT("emissive_intensity"), Intensity);
}