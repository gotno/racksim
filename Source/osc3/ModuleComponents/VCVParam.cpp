#include "ModuleComponents/VCVParam.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "osc3GameState.h"
#include "VCVModule.h"
#include "ModuleComponents/VCVLight.h"

#include "Misc/ScopeRWLock.h"
#include "Kismet/GameplayStatics.h"

AVCVParam::AVCVParam() {
  PrimaryActorTick.bCanEverTick = true;

  Tags.Add(TAG_INTERACTABLE_PARAM);

  // loading indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial>
    LoadingMaterialFinder(LoadingMaterialRef);
  if (LoadingMaterialFinder.Object)
    LoadingMaterialInterface = Cast<UMaterial>(LoadingMaterialFinder.Object);
}

void AVCVParam::BeginPlay() {
  Super::BeginPlay();
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  GameState = Cast<Aosc3GameState>(UGameplayStatics::GetGameState(this));

  if (LoadingMaterialInterface) {
    LoadingMaterialInstance = UMaterialInstanceDynamic::Create(LoadingMaterialInterface, this);
  }
}

void AVCVParam::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
}

void AVCVParam::Init(VCVParam* vcv_param) {
  Model = vcv_param; 
  Module = Cast<AVCVModule>(GetOwner());
  SetActorHiddenInGame(!Model->visible);
  SetActorEnableCollision(Model->visible);
  SetActorScale3D(FVector(RENDER_SCALE, Model->box.size.x, Model->box.size.y));
}

float AVCVParam::GetOverlapDelta() {
  return OverlapMesh->GetBounds().GetBox().GetSize().X * RENDER_SCALE;
}

void AVCVParam::Update(VCVParam& vcv_param) {
  FScopeLock Lock(&DataGuard);
  Model->merge(vcv_param);
  SetActorHiddenInGame(!Model->visible);
  SetActorEnableCollision(Model->visible);
}

FString AVCVParam::GetModuleBrand() {
  return Module->Brand;
}

void AVCVParam::GetTooltipText(FString& Label, FString& DisplayValue) {
  FScopeLock Lock(&DataGuard);
  Label = Model->name;
  DisplayValue = Model->displayValue;
}

void AVCVParam::SpawnLights(USceneComponent* AttachTo) {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  for (TPair<int32, VCVLight>& light_kvp : Model->Lights) {
    VCVLight& light = light_kvp.Value;

    FVector lightLocation = AttachTo->GetComponentLocation();

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      lightLocation,
      FRotator(0, 0, 0),
      spawnParams
    );
    a_light->AttachToComponent(AttachTo, FAttachmentTransformRules::KeepWorldTransform);
    a_light->Init(&light);
    Module->RegisterParamLight(light.id, a_light);
  }
}

void AVCVParam::SetValue(float inValue) {
  float roundedValue = FMath::RoundHalfToEven(inValue * 1000) / 1000;
  if (roundedValue == Model->value) return;

  FScopeLock Lock(&DataGuard);
  Model->value = roundedValue;
  Module->ParamUpdated(Model->id, Model->value);
}

void AVCVParam::Engage() {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  bEngaged = true;
  OldValue = Model->value;
}

void AVCVParam::Engage(float _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  bEngaged = true;
  OldValue = Model->value;
}

void AVCVParam::Engage(FVector _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param engage"));
  bEngaged = true;
  OldValue = Model->value;
}

void AVCVParam::Alter(float amount) {
  // UE_LOG(LogTemp, Warning, TEXT("param alter %f, ratio'd: %f"), amount, amount * AlterRatio);
}

void AVCVParam::Alter(FVector _value) {
  // UE_LOG(LogTemp, Warning, TEXT("param alter %s, ratio'd: %s"), *_value.ToCompactString(), *(_value * AlterRatio).ToCompactString());
}

void AVCVParam::Release() {
  // UE_LOG(LogTemp, Warning, TEXT("param release"));
  bEngaged = false;
  GameMode->RequestModuleDiff(Module->Id);
  if (OldValue != Model->value) GameState->SetUnsaved();
}

void AVCVParam::ResetValue() {
  // UE_LOG(LogTemp, Warning, TEXT("param reset"));
  if (Model) SetValue(Model->defaultValue);
}