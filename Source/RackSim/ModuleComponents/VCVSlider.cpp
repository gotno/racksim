#include "ModuleComponents/VCVSlider.h"

#include "osc3.h"
#include "VCVModule.h"
#include "VCVData/VCV.h"
#include "osc3GameModeBase.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVSlider::AVCVSlider() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));

  // base mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBase(BaseMeshReference);
  if (MeshBase.Object) BaseMeshComponent->SetStaticMesh(MeshBase.Object);
  SetRootComponent(BaseMeshComponent);

  // base materials
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseFaceMaterial(BaseFaceMaterialReference);
  if (BaseFaceMaterial.Object) {
    BaseFaceMaterialInterface = Cast<UMaterial>(BaseFaceMaterial.Object);
  }

  HandleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle Mesh"));
  HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  
  // handle mesh
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshHandle(HandleMeshReference);
  if (MeshHandle.Object) {
    HandleMeshComponent->SetStaticMesh(MeshHandle.Object);
    OverlapMesh = MeshHandle.Object;
  }
  HandleMeshComponent->SetupAttachment(GetRootComponent());

  BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BaseMeshComponent->SetCollisionObjectType(PARAM_OBJECT);

  HandleMeshComponent->SetGenerateOverlapEvents(true);
  HandleMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  HandleMeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  HandleMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  HandleMeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);

  // handle materials
  static ConstructorHelpers::FObjectFinder<UMaterial> HandleMaterial(HandleMaterialReference);
  if (HandleMaterial.Object) {
    HandleMaterialInterface = Cast<UMaterial>(HandleMaterial.Object);
  }
  static ConstructorHelpers::FObjectFinder<UMaterial> HandleFaceMaterial(HandleFaceMaterialReference);
  if (HandleFaceMaterial.Object) {
    HandleFaceMaterialInterface = Cast<UMaterial>(HandleFaceMaterial.Object);
  }
  
  SetActorEnableCollision(true);
}

void AVCVSlider::BeginPlay() {
  Super::BeginPlay();

  if (LoadingMaterialInterface) {
    BaseMeshComponent->SetMaterial(1, LoadingMaterialInstance);
    HandleMeshComponent->SetMaterial(1, LoadingMaterialInstance);
  }

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (BaseFaceMaterialInterface) {
    BaseFaceMaterialInstance = UMaterialInstanceDynamic::Create(BaseFaceMaterialInterface, this);
  }

  if (HandleMaterialInterface) {
    HandleMaterialInstance = UMaterialInstanceDynamic::Create(HandleMaterialInterface, this);
    HandleMeshComponent->SetMaterial(0, HandleMaterialInstance);
    HandleMaterialInstance->SetVectorParameterValue(FName("Color"), FColor::Black);
  }
  if (HandleFaceMaterialInterface) {
    HandleFaceMaterialInstance = UMaterialInstanceDynamic::Create(HandleFaceMaterialInterface, this);
  }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVSlider::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
}

  void AVCVSlider::Init(VCVParam* vcv_param) {
    Super::Init(vcv_param);

  HandleMeshComponent->SetWorldScale3D(FVector(1.f, Model->handleBox.size.x, Model->handleBox.size.y));
  SpawnLights(HandleMeshComponent);

  FVector minHandlePosition = GetActorLocation() + FVector(0, Model->minHandlePos.x, Model->minHandlePos.y);
  WorldOffsetPercent = GetOffsetPercentFromValue(true);
  HandleMeshComponent->SetWorldLocation(minHandlePosition + GetSliderDirectionVector() * GetWorldOffset(true));
  ShadowOffsetPercent = WorldOffsetPercent;

  for (FString& svgPath : Model->svgPaths) {
    if (!svgPath.IsEmpty())
      GameMode->RequestTexture(svgPath, this, FName("SetTexture"));
  }
}

void AVCVSlider::SetTexture(FString Filepath, UTexture2D* inTexture) {
  bool bAnyTextureSet{false};
  if (!BaseTexture && !Model->svgPaths[0].IsEmpty() && Filepath.Equals(Model->svgPaths[0])) {
    BaseTexture = inTexture;
    bAnyTextureSet = true;
    BaseFaceMaterialInstance->SetTextureParameterValue(FName("texture"), BaseTexture);
  }
  if (!HandleTexture && !Model->svgPaths[1].IsEmpty() && Filepath.Equals(Model->svgPaths[1])) {
    HandleTexture = inTexture;
    bAnyTextureSet = true;
    HandleFaceMaterialInstance->SetTextureParameterValue(FName("texture"), HandleTexture);
  }
  if (bAnyTextureSet) {
    BaseMeshComponent->SetMaterial(1, BaseFaceMaterialInstance);
    HandleMeshComponent->SetMaterial(1, HandleFaceMaterialInstance);
  }
}

FVector AVCVSlider::GetSliderDirectionVector() {
  return Model->horizontal
    ? HandleMeshComponent->GetRightVector()
    : HandleMeshComponent->GetUpVector();
}

float AVCVSlider::GetMaxOffset(bool bUnscaled) {
  float unscaledOffset =
    Model->horizontal
      ? Model->maxHandlePos.x - Model->minHandlePos.x
      : Model->maxHandlePos.y - Model->minHandlePos.y;

  return bUnscaled ? unscaledOffset : unscaledOffset * AVCVModule::Scale;
}

float AVCVSlider::GetWorldOffset(bool bUnscaled) {
  return WorldOffsetPercent * GetMaxOffset(bUnscaled);
}

float AVCVSlider::GetOffsetPercentFromValue(bool bUnscaled) {
  float valuePercent = (Model->value - Model->minValue) / (Model->maxValue - Model->minValue);
  return (valuePercent * GetMaxOffset(bUnscaled)) / GetMaxOffset(bUnscaled);
}

float AVCVSlider::GetValueFromOffset() {
  float value =
    Model->minValue + (ShadowOffsetPercent * (Model->maxValue - Model->minValue));
  if (Model->snap) value = round(value);
  return value;
}

void AVCVSlider::Engage(FVector ControllerPosition) {
  Super::Engage();

  LastControllerPosition = ControllerPosition;
  LastPositionDelta = 0.f;
  LastValue = Model->value;
}

void AVCVSlider::Alter(FVector ControllerPosition) {
  Super::Alter(ControllerPosition);
  if (!bEngaged) return;

  FVector sliderDirectionVector = GetSliderDirectionVector();
  FVector controllerVector = ControllerPosition - LastControllerPosition; 

  float positionDelta = FVector::DotProduct(controllerVector, sliderDirectionVector);
  positionDelta *= AlterRatio;
  positionDelta =
    FMath::WeightedMovingAverage(positionDelta, LastPositionDelta, 0.2f);
  LastPositionDelta = positionDelta;

  ShadowOffsetPercent =
    FMath::Clamp(
      ShadowOffsetPercent * GetMaxOffset() + positionDelta,
      0.f,
      GetMaxOffset()
    ) / GetMaxOffset();

  SetValue(GetValueFromOffset());

  FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - sliderDirectionVector * GetWorldOffset();
  WorldOffsetPercent = Model->snap ? GetOffsetPercentFromValue() : ShadowOffsetPercent;
  HandleMeshComponent->SetWorldLocation(zeroPosition + sliderDirectionVector * GetWorldOffset());

  LastControllerPosition = ControllerPosition;
}

void AVCVSlider::Release() {
  // treat snapping sliders like a switch and increment
  // if they haven't been dragged enough to change value
  if (Model->snap && Model->value == LastValue) {
    float newValue = Model->value + 1;
    if (newValue > Model->maxValue) newValue = 0;
    SetValue(newValue);
    
    FVector sliderDirectionVector = GetSliderDirectionVector();

    FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - sliderDirectionVector * GetWorldOffset();
    WorldOffsetPercent = GetOffsetPercentFromValue();
    HandleMeshComponent->SetWorldLocation(zeroPosition + sliderDirectionVector * GetWorldOffset());
  }

  LastValue = Model->value;
  ShadowOffsetPercent = WorldOffsetPercent;

  Super::Release();
}

void AVCVSlider::ResetValue() {
  Super::ResetValue();

  FVector sliderDirectionVector = GetSliderDirectionVector();
  FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - sliderDirectionVector * GetWorldOffset();

  WorldOffsetPercent = GetOffsetPercentFromValue();
  HandleMeshComponent->SetWorldLocation(zeroPosition + sliderDirectionVector * GetWorldOffset());
  ShadowOffsetPercent = WorldOffsetPercent;
}

void AVCVSlider::Update(VCVParam& vcv_param) {
  Super::Update(vcv_param);

  FVector sliderDirectionVector = GetSliderDirectionVector();
  FVector zeroPosition = HandleMeshComponent->GetComponentLocation() - sliderDirectionVector * GetWorldOffset();

  WorldOffsetPercent = GetOffsetPercentFromValue();
  HandleMeshComponent->SetWorldLocation(zeroPosition + sliderDirectionVector * GetWorldOffset());
  ShadowOffsetPercent = WorldOffsetPercent;
}