#include "VCVCable.h"

// #include "osc3GameModeBase.h"
// #include "VCV.h"

// #include "CableComponent.h"
// #include "Kismet/GameplayStatics.h"
// #include "Kismet/KismetMathLibrary.h"
// #include "Math/UnrealMathUtility.h"

AVCVCable::AVCVCable() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
  SetRootComponent(RootSceneComponent);

  InputMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Input Mesh"));
  // InputMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  // InputMeshComponent->SetMobility(EComponentMobility::Movable);

  OutputMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Output Mesh"));
  // OutputMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  // OutputMeshComponent->SetMobility(EComponentMobility::Movable);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> JackBody(JackMeshReference);
  if (JackBody.Object) {
    InputMeshComponent->SetStaticMesh(JackBody.Object);
    OutputMeshComponent->SetStaticMesh(JackBody.Object);

    FVector scale{RENDER_SCALE * 0.8};
    InputMeshComponent->SetWorldScale3D(scale);
    OutputMeshComponent->SetWorldScale3D(scale);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  
  // CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Component"));
  // CableComponent->bSkipCableUpdateWhenNotOwnerRecentlyRendered = true;
  // CableComponent->SetupAttachment(InputMeshComponent, TEXT("wire"));
  // CableComponent->SetAttachEndToComponent(OutputMeshComponent, TEXT("wire"));
  // CableComponent->EndLocation = FVector(0.f);
  // CableComponent->CableWidth = 0.3f;
  // CableComponent->bAttachStart = true;
  // CableComponent->bAttachEnd = true;
  // CableComponent->SetEnableGravity(false);
  // CableComponent->CableForce = FVector(0.f, 0.f, 0.f);
  // CableComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  // CableComponent->bEnableStiffness = false;
  // CableComponent->NumSegments = 16;
  // CableComponent->NumSides = 8;
  // CableComponent->SolverIterations = 16;
  // CableComponent->CableLength = 20.f;
}

// void AVCVCable::SetAlive(bool inAlive) {
//   bAlive = inAlive;

//   if (bAlive) {
//     CableComponent->SetComponentTickEnabled(true);
//   } else {
//     CableComponent->SetComponentTickEnabled(false);
//   }
// }

// void AVCVCable::Sleep() {
//   SetAlive(false);
// }

// void AVCVCable::Wake() {
//   SetAlive(true);
// }

void AVCVCable::BeginPlay() {
	Super::BeginPlay();

  cableColor = cableColors[FMath::RandRange(0, cableColors.Num() - 1)];

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);

    InputMeshComponent->SetMaterial(0, BaseMaterialInstance);
    OutputMeshComponent->SetMaterial(0, BaseMaterialInstance);
    // CableComponent->SetMaterial(0, BaseMaterialInstance);

    BaseMaterialInstance->SetVectorParameterValue(FName("Color"), cableColor);
  }

  // FTimerHandle sleepHandle;
  // GetWorld()->GetTimerManager().SetTimer(
  //   sleepHandle,
  //   this,
  //   &AVCVCable::Sleep,
  //   0.01f, // seconds, apparently
  //   false
  // );
}

void AVCVCable::UnsetPort(PortType Type) {
  Ports[Type]->RemoveCable(this);
  Ports.Remove(Type);
  // HandlePortChange();
}

void AVCVCable::SetPort(AVCVPort* Port) {
  Ports.Add(Port->Type, Port);
  Port->AddCable(this);
  // HandlePortChange();
}

void AVCVCable::HandlePortChange() {
  /* if (IsComplete() && !id somehow) { */
  /*   request persist */
  /* } else if (id somehow && !IsComplete()) { */
  /*   request destroy */
  /* } */
}

// void AVCVCable::init(VCVCable vcv_cable) {
//   model = vcv_cable; 
//   draw();
//   // UE_LOG(LogTemp, Warning, TEXT("cable model %lld: %lld:%lld"), model.id, model.inputModuleId, model.outputModuleId);
// }

void AVCVCable::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

// void AVCVCable::disconnectFrom(PortIdentity identity) {
//   model.nullifyIdentity(identity.type);
// }

// void AVCVCable::connectTo(PortIdentity identity) {
//   if (model.portIdentities[identity.type].isNull()) {
//     model.portIdentities[identity.type] = identity;
//   }
// }

// void AVCVCable::setHangingLocation(FVector _hangingLocation, FVector _hangingForwardVector) {
//   hangingLocation = _hangingLocation;
//   hangingForwardVector = _hangingForwardVector;
// }

PortType AVCVCable::GetHangingType() {
  checkf(!IsComplete(), TEXT("cable IsComplete and has no hanging type"));

  if (Ports.Contains(PortType::Input) return PortType::Output;
  return PortType::Input;
}

// PortIdentity AVCVCable::getConnectedPortIdentity() {
//   if (model.portIdentities[PortType::Input].isNull()) return model.portIdentities[PortType::Output];
//   return model.portIdentities[PortType::Input];
// }

// void AVCVCable::setId(int64_t& inId) {
//   model.id = inId;
// }

// int64_t AVCVCable::getId() {
//   return model.id;
// }

// VCVCable AVCVCable::getModel() {
//   return model;
// }

void AVCVCable::draw() {
  Aosc3GameModeBase* gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

  FVector inputLocation, outputLocation, inputForwardVector, outputForwardVector;

  if (model.portIdentities[PortType::Input].isNull()) {
    inputLocation = hangingLocation;
    inputForwardVector = hangingForwardVector;
  } else {
    gameMode->GetPortInfo(
      model.portIdentities[PortType::Input],
      inputLocation,
      inputForwardVector
    );
  }
  if (model.portIdentities[PortType::Output].isNull()) {
    outputLocation = hangingLocation;
    outputForwardVector = hangingForwardVector;
  } else {
    gameMode->GetPortInfo(
      model.portIdentities[PortType::Output],
      outputLocation,
      outputForwardVector
    );
  }

  float distanceBetweenEnds = FVector::Distance(inputLocation, outputLocation);
  CableComponent->CableLength = distanceBetweenEnds * 1.2;
  CableComponent->CableForce = (inputForwardVector + outputForwardVector) * 0.5f * -1000.f;
  InputMeshComponent->SetWorldLocation(inputLocation);
  InputMeshComponent->SetWorldRotation(inputForwardVector.Rotation());
  OutputMeshComponent->SetWorldLocation(outputLocation);
  OutputMeshComponent->SetWorldRotation(outputForwardVector.Rotation());
}
