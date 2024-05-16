#include "ModuleComponents/VCVSwitch.h"

#include "osc3.h"
#include "VCVData/VCV.h"
#include "osc3GameModeBase.h"

#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVSwitch::AVCVSwitch() {
  MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
  SetRootComponent(MeshComponent);

  MeshComponent->SetGenerateOverlapEvents(true);
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  MeshComponent->SetCollisionObjectType(PARAM_OBJECT);
  MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  MeshComponent->SetCollisionResponseToChannel(LIGHT_OBJECT, ECollisionResponse::ECR_Overlap);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(MeshReference);
  if (Mesh.Object) MeshComponent->SetStaticMesh(Mesh.Object);

  // base material
  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  // face material
  static ConstructorHelpers::FObjectFinder<UMaterial> FaceMaterial(FaceMaterialReference);
  if (FaceMaterial.Object) FaceMaterialInterface = Cast<UMaterial>(FaceMaterial.Object);

  SetActorEnableCollision(true);
}

void AVCVSwitch::BeginPlay() {
	Super::BeginPlay();

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    MeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FColor(181, 170, 169, 255));
  }
  if (FaceMaterialInterface) {
    FaceMaterialInstance = UMaterialInstanceDynamic::Create(FaceMaterialInterface, this);
    MeshComponent->SetMaterial(1, FaceMaterialInstance);
  }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVSwitch::Tick(float DeltaTime) {
}

void AVCVSwitch::Init(VCVParam* vcv_param) {
	Super::Init(vcv_param);
  
  // remove empty svg paths and init frames array to same size
  vcv_param->svgPaths.Remove(FString(""));
  Frames.Init(nullptr, vcv_param->svgPaths.Num());

  for (FString& svgPath : Model->svgPaths) {
    GameMode->RequestTexture(svgPath, this, FName("SetTexture"));
  }
}

void AVCVSwitch::SetTexture(FString Filepath, UTexture2D* inTexture) {
  int frameIndex = 0;
  for (UTexture2D* frameTexture : Frames) {
    if (!Frames[frameIndex] && Filepath.Equals(Model->svgPaths[frameIndex])) {
      Frames[frameIndex] = inTexture;
      if (GetFrameFromValue() == frameIndex) SetFrame();
    }
    ++frameIndex;
  }
}

void AVCVSwitch::Engage() {
  Super::Engage();
  float newValue = Model->value + 1;
  if (newValue > Model->maxValue) newValue = Model->minValue;
  SetValue(newValue);
  SetFrame();
}

// some values don't align with frame index
// (looking at you, instruo)
// get the distance from min value instead
int AVCVSwitch::GetFrameFromValue() {
  return Model->value - Model->minValue;
}

void AVCVSwitch::SetFrame() {
  int frame = GetFrameFromValue();
  if (frame >= 0 && frame < Frames.Num())
    FaceMaterialInstance->SetTextureParameterValue(FName("texture"), Frames[frame]);
}

void AVCVSwitch::ResetValue() {
  Super::ResetValue();
  SetFrame();
}

void AVCVSwitch::Update(VCVParam& vcv_param) {
  Super::Update(vcv_param);
  SetFrame();
}