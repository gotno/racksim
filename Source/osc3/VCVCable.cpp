#include "VCV.h"
#include "VCVCable.h"
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
  hangingType = identity.type;
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
  return hangingType;
}

int64_t AVCVCable::getId() {
  return model.id;
}

void AVCVCable::draw() {
  if (model.id == -1) return;
  Aosc3GameModeBase* gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

  // bool inputModulePresent, outputModulePresent;
  FVector inputLocation, outputLocation, inputForwardVector, outputForwardVector;

  if (model.portIdentities[PortType::Input].isNull()) {
    // UE_LOG(LogTemp, Warning, TEXT("input is nullified"));
    inputLocation = hangingLocation;
    inputForwardVector = hangingForwardVector;
  } else {
  // inputModulePresent =
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
  // outputModulePresent =
    gameMode->GetPortInfo(
      model.portIdentities[PortType::Output],
      outputLocation,
      outputForwardVector
    );
  }
  
  // if (!inputModulePresent || !outputModulePresent) {
  //   // UE_LOG(LogTemp, Warning, TEXT("module missing for cable %lld"), model.id);
  //   return;
  // } else {
  //   // UE_LOG(LogTemp, Warning, TEXT("module found for cable %lld"), model.id);
  // }

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

  DrawDebugCircle(
    GetWorld(),
    inputTransform.ToMatrixNoScale(),
    plugRadius,
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
    inputTranslation + directionUnitVector * plugRadius,
    outputTranslation + -directionUnitVector * plugRadius,
    cableColor,
    false,
    -1.f,
    0,
    lineWeight
  );
  DrawDebugCircle(
    GetWorld(),
    outputTransform.ToMatrixNoScale(),
    plugRadius,
    64,
    cableColor,
    false,
    -1.f,
    0,
    lineWeight,
    false
  );
}