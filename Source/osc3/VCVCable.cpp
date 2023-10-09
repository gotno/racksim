#include "VCVCable.h"

#include "osc3.h"
#include "VCV.h"
#include "osc3GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AVCVCable::AVCVCable() {
	PrimaryActorTick.bCanEverTick = true;

  USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
  SetRootComponent(root);
}

void AVCVCable::BeginPlay() {
	Super::BeginPlay();
}

void AVCVCable::init(VCVCable vcv_cable) {
  model = vcv_cable; 
  // UE_LOG(LogTemp, Warning, TEXT("cable model %lld: %lld:%lld"), model.id, model.inputModuleId, model.outputModuleId);
}

void AVCVCable::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  draw();
}

void AVCVCable::disconnectFrom(PortIdentity identity) {
  model.nullifyIdentity(identity.type);
}

void AVCVCable::connectTo(PortIdentity identity) {
  if (model.portIdentities[identity.type].isNull()) {
    model.portIdentities[identity.type] = identity;
  }
}

void AVCVCable::setHangingLocation(FVector _hangingLocation, FVector _hangingForwardVector) {
  hangingLocation = _hangingLocation;
  hangingForwardVector = _hangingForwardVector;
}

PortType AVCVCable::getHangingType() {
  if (model.portIdentities[PortType::Input].isNull()) return PortType::Input;
  return PortType::Output;
}

int64_t AVCVCable::getId() {
  return model.id;
}

VCVCable AVCVCable::getModel() {
  return model;
}

void AVCVCable::draw() {
  Aosc3GameModeBase* gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

  FVector inputLocation, outputLocation, inputForwardVector, outputForwardVector;

  if (model.portIdentities[PortType::Input].isNull()) {
    // UE_LOG(LogTemp, Warning, TEXT("input is nullified"));
    inputLocation = hangingLocation;
    inputForwardVector = hangingForwardVector;
  } else {
    // UE_LOG(LogTemp, Warning, TEXT("input not nullified"));
    gameMode->GetPortInfo(
      model.portIdentities[PortType::Input],
      inputLocation,
      inputForwardVector
    );
  }
  if (model.portIdentities[PortType::Output].isNull()) {
    // UE_LOG(LogTemp, Warning, TEXT("output is nullified"));
    outputLocation = hangingLocation;
    outputForwardVector = hangingForwardVector;
  } else {
    // UE_LOG(LogTemp, Warning, TEXT("output not nullified"));
    gameMode->GetPortInfo(
      model.portIdentities[PortType::Output],
      outputLocation,
      outputForwardVector
    );
  }

  FTransform inputTransform, outputTransform;
  inputTransform.SetIdentity();
  outputTransform.SetIdentity();

  FVector inputTranslation = inputLocation + (inputForwardVector * plugOffset);
  FVector outputTranslation = outputLocation + (outputForwardVector * plugOffset);
  FVector directionUnitVector =
    UKismetMathLibrary::GetDirectionUnitVector(inputTranslation, outputTranslation);

  inputTransform.SetTranslation(inputTranslation);
  inputTransform.SetRotation(inputForwardVector.Rotation().Quaternion());
  outputTransform.SetTranslation(outputTranslation);
  outputTransform.SetRotation(outputForwardVector.Rotation().Quaternion());

  if (model.id == -1) cableColor = FColor::White;

  DrawDebugCircle(
    GetWorld(),
    inputTransform.ToMatrixNoScale(),
    plugRadius * RENDER_SCALE,
    64,
    cableColor,
    false,
    -1.f,
    0,
    lineWeight,
    false
  );
  DrawDebugLine(
    GetWorld(),
    inputTranslation + directionUnitVector * (plugRadius * RENDER_SCALE),
    outputTranslation + -directionUnitVector * (plugRadius * RENDER_SCALE),
    cableColor,
    false,
    -1.f,
    0,
    lineWeight
  );
  DrawDebugCircle(
    GetWorld(),
    outputTransform.ToMatrixNoScale(),
    plugRadius * RENDER_SCALE,
    64,
    cableColor,
    false,
    -1.f,
    0,
    lineWeight,
    false
  );
}