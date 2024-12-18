#include "ModuleComponents/VCVKnob.h"

#include "osc3GameModeBase.h"
#include "VCVData/VCVOverrides.h"

#include "engine/Texture2D.h"

#include "UObject/ConstructorHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/BodySetup.h"

AVCVKnob::AVCVKnob() {
  BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  RootComponent = BaseMeshComponent;

  BaseMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  BaseMeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(MeshReference);
  
  if (MeshBody.Object) {
    BaseMeshComponent->SetStaticMesh(MeshBody.Object);
    OverlapMesh = MeshBody.Object;
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(FaceMaterialReference);
  
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
  if (FaceMaterial.Object) {
    FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);
  }
}

void AVCVKnob::BeginPlay() {
  Super::BeginPlay();

  UBodySetup* bodySetup = BaseMeshComponent->GetBodySetup();
  if (bodySetup) bodySetup->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;

  if (LoadingMaterialInterface) {
    BaseMeshComponent->SetMaterial(1, LoadingMaterialInstance);
  }

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    BaseMeshComponent->SetMaterial(0, BaseMaterialInstance);
  }
  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
  }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVKnob::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
}

void AVCVKnob::Init(VCVParam* vcv_param) {
  Super::Init(vcv_param);

  Gap = (180 - FMath::Abs(Model->maxAngle)) + (180 - FMath::Abs(Model->minAngle));

  UpdateRotation(GetRotationFromValue());

  VCVOverrides overrides;
  FVector scaleMultiplier = overrides.getScaleMultiplier(GetModuleBrand(), Model->svgPaths[1]);
  SetActorScale3D(GetActorScale3D() * scaleMultiplier);

  BaseMaterialInstance->SetVectorParameterValue(FName("color"), Model->bodyColor);

  for (FString& svgPath : Model->svgPaths) {
    if (!svgPath.IsEmpty())
      GameMode->RequestTexture(svgPath, this, FName("SetTexture"));
  }
}

void AVCVKnob::SetTexture(FString Filepath, UTexture2D* inTexture) {
  bool bAnyTextureSet{false};
  if (!TextureBackground && !Model->svgPaths[0].IsEmpty() && Filepath.Equals(Model->svgPaths[0])) {
    TextureBackground = inTexture;
    bAnyTextureSet = true;
    FaceMaterialInstance->SetTextureParameterValue(FName("texture_bg"), TextureBackground);
  }
  if (!Texture && !Model->svgPaths[1].IsEmpty() && Filepath.Equals(Model->svgPaths[1])) {
    Texture = inTexture;
    bAnyTextureSet = true;
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);
  }
  if (!TextureForeground && !Model->svgPaths[2].IsEmpty() && Filepath.Equals(Model->svgPaths[2])) {
    TextureForeground = inTexture;
    bAnyTextureSet = true;
    FaceMaterialInstance->SetTextureParameterValue(FName("texture_fg"), TextureForeground);
  }
  if (bAnyTextureSet) BaseMeshComponent->SetMaterial(1, FaceMaterialInstance);
}

void AVCVKnob::UpdateRotation(FRotator inRotation) {
  FaceMaterialInstance->SetScalarParameterValue(FName("rotation"), -inRotation.Roll);
  BaseMeshComponent->SetRelativeRotation(inRotation);
}

FRotator AVCVKnob::GetRotationFromValue() {
  float valuePercent = (Model->value - Model->minValue) / (Model->maxValue - Model->minValue);
  float valueAngle = Model->minAngle + valuePercent * (Model->maxAngle - Model->minAngle);

  FRotator knobRotation = BaseMeshComponent->GetRelativeRotation();
  return FRotator(knobRotation.Pitch, knobRotation.Yaw, valueAngle);
}

float AVCVKnob::GetValueFromRotation() {
  float roll = ShadowRotation.Roll;
  float rotationPercent = (roll - Model->minAngle) / (Model->maxAngle - Model->minAngle);
  float value = Model->minValue + ((Model->maxValue - Model->minValue) * rotationPercent);

  if (Model->snap) value = round(value);
  return value;
}

void AVCVKnob::Engage(float ControllerRoll) {
  Super::Engage(ControllerRoll);
  ShadowRotation = BaseMeshComponent->GetRelativeRotation();
  LastControllerRoll = ControllerRoll;
}

void AVCVKnob::Alter(float ControllerRoll, float AlterRatioOverride) {
  float originalRatio = AlterRatio;
  AlterRatio = AlterRatioOverride;

  Alter(ControllerRoll);

  AlterRatio = originalRatio;
}

void AVCVKnob::Alter(float ControllerRoll) {
  Super::Alter(ControllerRoll);
  if (!bEngaged) return;

  float deltaControllerRoll = ControllerRoll - LastControllerRoll;
  if (FMath::Abs(deltaControllerRoll) > Gap) {
    // prevent jump from min to max or max to min
    LastControllerRoll = ControllerRoll;
    return;
  }

  float deltaRoll = deltaControllerRoll * AlterRatio;
  deltaRoll =
    FMath::WeightedMovingAverage(deltaRoll, LastDeltaRoll, MovingAverageWeight);
  LastDeltaRoll = deltaRoll;

  FRotator knobRotation = BaseMeshComponent->GetRelativeRotation();
  ShadowRotation.Pitch = knobRotation.Pitch;
  ShadowRotation.Yaw = knobRotation.Yaw;
  ShadowRotation.Roll =
    FMath::Clamp(ShadowRotation.Roll + deltaRoll, Model->minAngle, Model->maxAngle);

  SetValue(GetValueFromRotation());
  UpdateRotation(Model->snap ? GetRotationFromValue() : ShadowRotation);

  LastControllerRoll = ControllerRoll;
}

void AVCVKnob::Release() {
  Super::Release();
}

void AVCVKnob::ResetValue() {
  Super::ResetValue();
  UpdateRotation(GetRotationFromValue());
}

void AVCVKnob::Update(VCVParam& param) {
  Super::Update(param);
  UpdateRotation(GetRotationFromValue());
}