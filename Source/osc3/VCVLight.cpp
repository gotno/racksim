#include "VCVLight.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "VCV.h"
#include "VCVModule.h"

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
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVLight::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVLight::init(VCVLight* vcv_light) {
  model = vcv_light;
  
  if (model->shape == LightShape::Round) {
    BaseMeshComponent->SetStaticMesh(circleMesh);
  } else {
    BaseMeshComponent->SetStaticMesh(rectangleMesh);
  }

  SetColor(model->bgColor);
  SetEmissiveColor(model->color);
  SetEmissiveIntensity(model->color.A);

  SetHidden(!model->visible);
  SetActorEnableCollision(model->visible);
  
  SetActorScale3D(FVector(0.01f, model->box.size.x, model->box.size.y));
}

void AVCVLight::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (bHandledOverlap) return;

  AVCVModule* ownerModule = Cast<AVCVModule>(GetOwner());
  ownerModule = ownerModule ? ownerModule : Cast<AVCVModule>(GetOwner()->GetOwner());
  if (ownerModule != OtherActor->GetOwner()) return;
  // UE_LOG(LogTemp, Warning, TEXT("handling overlap of %s:%s/%s"), *ownerModule->getBrand(), *GetActorNameOrLabel(), *OtherActor->GetActorNameOrLabel());

  FVector _, otherActorExtent;
  OtherActor->GetActorBounds(false, _, otherActorExtent);
  AddActorLocalOffset(FVector(-otherActorExtent.X, 0.f, 0.f));

  bHandledOverlap = true;
}

void AVCVLight::SetColor(FLinearColor color) {
  BaseMaterialInstance->SetVectorParameterValue(TEXT("color"), color);
}

void AVCVLight::SetEmissiveColor(FLinearColor color) {
  if (color.A == 0.f) color = FLinearColor(0.f, 0.f, 0.f, 0.f);
  BaseMaterialInstance->SetVectorParameterValue(TEXT("emissive_color"), color);
}

void AVCVLight::SetEmissiveIntensity(float intensity) {
  BaseMaterialInstance->SetScalarParameterValue(TEXT("emissive_intensity"), intensity);
}