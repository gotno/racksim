#include "VCVCable.h"

#include "osc3GameModeBase.h"
#include "ModuleComponents/VCVPort.h"

#include "CableComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AVCVCable::AVCVCable() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
  SetRootComponent(RootSceneComponent);

  InputMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Input Mesh"));
  OutputMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Output Mesh"));

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

  static ConstructorHelpers::FObjectFinder<UMaterial> CableMaterial(CableMaterialReference);
  if (CableMaterial.Object) CableMaterialInterface = Cast<UMaterial>(CableMaterial.Object);
  
  CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Component"));
  CableComponent->SetupAttachment(InputMeshComponent, TEXT("wire"));
  CableComponent->SetAttachEndToComponent(OutputMeshComponent, TEXT("wire"));
  CableComponent->EndLocation = FVector(0.f);
  CableComponent->CableWidth = 0.3f;
  CableComponent->bAttachStart = true;
  CableComponent->bAttachEnd = true;
  CableComponent->SetEnableGravity(false);
  CableComponent->CableForce = FVector(0.f, 0.f, 0.f);
  CableComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  CableComponent->bEnableStiffness = false;
  CableComponent->NumSegments = 32;
  CableComponent->NumSides = 6;
  CableComponent->SolverIterations = 16;
}

void AVCVCable::Sleep() {
  CableComponent->SetComponentTickEnabled(false);
}

void AVCVCable::Wake() {
  CableComponent->SetComponentTickEnabled(true);
}

void AVCVCable::BeginPlay() {
	Super::BeginPlay();

  CableColor = CableColors[CurrentCableColorIndex++];
  if (CurrentCableColorIndex > CableColors.Num() - 1) CurrentCableColorIndex = 0;

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);

    InputMeshComponent->SetMaterial(0, BaseMaterialInstance);
    OutputMeshComponent->SetMaterial(0, BaseMaterialInstance);

    BaseMaterialInstance->SetVectorParameterValue(FName("Color"), CableColor);
  }

  if (CableMaterialInterface) {
    CableMaterialInstance = UMaterialInstanceDynamic::Create(CableMaterialInterface, this);
    CableComponent->SetMaterial(0, CableMaterialInstance);
    CableMaterialInstance->SetVectorParameterValue(FName("Color"), CableColor);
    CableMaterialInstance->SetScalarParameterValue(FName("Opacity"), CABLE_OPACITY);
  }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
  Ports.Reserve(2);
}

void AVCVCable::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

  for (auto& pair : Ports) {
    pair.Value->RemoveCable(this);
  }
}

void AVCVCable::SetPort(AVCVPort* Port) {
  Ports.Add(Port->Type, Port);
  Port->AddCable(this);
  HandlePortChange();
  
  UpdateEndPositions();
}

void AVCVCable::UnsetPort(PortType Type) {
  Ports[Type]->RemoveCable(this);
  Ports.Remove(Type);
  HandlePortChange();
  
  UpdateEndPositions();
}

AVCVPort* AVCVCable::GetPort(PortType Type) {
  if (Ports.Contains(Type)) return Ports[Type];
  return nullptr;
}

void AVCVCable::HandlePortChange() {
  if (IsComplete() && !IsRegistered()) {
    GameMode->RegisterCableConnect(Ports[PortType::Input], Ports[PortType::Output]);
    return;
  }
  
  if (IsRegistered() && !IsComplete()) {
    GameMode->RegisterCableDisconnect(this);
    return;
  }
}

void AVCVCable::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVCable::SetHangingEndLocation(FVector inHangingLocation, FVector inHangingForwardVector) {
  HangingLocation = inHangingLocation;
  HangingForwardVector = inHangingForwardVector;
  
  UpdateEndPositions();
}

PortType AVCVCable::GetHangingType() {
  checkf(!IsComplete(), TEXT("cable IsComplete and has no hanging type"));

  if (Ports.Contains(PortType::Input)) return PortType::Output;
  return PortType::Input;
}

void AVCVCable::UpdateEndPositions() {
  FVector inputLocation = Ports.Contains(PortType::Input)
    ? Ports[PortType::Input]->GetActorLocation()
    : HangingLocation;
  FVector inputForwardVector = Ports.Contains(PortType::Input)
    ? Ports[PortType::Input]->GetActorForwardVector()
    : HangingForwardVector;
  FVector outputLocation = Ports.Contains(PortType::Output)
    ? Ports[PortType::Output]->GetActorLocation()
    : HangingLocation;
  FVector outputForwardVector = Ports.Contains(PortType::Output)
    ? Ports[PortType::Output]->GetActorForwardVector()
    : HangingForwardVector;

  float distanceBetweenEnds = FVector::Distance(inputLocation, outputLocation);
  CableComponent->CableLength = distanceBetweenEnds * 0.8;

  // naive attempt to set cable force relative to average of cable end vectors
  // actually working kind of ok!
  CableComponent->CableForce = (inputForwardVector + outputForwardVector) * 0.5f * -500.f;

  InputMeshComponent->SetWorldLocation(inputLocation);
  InputMeshComponent->SetWorldRotation(inputForwardVector.Rotation());
  OutputMeshComponent->SetWorldLocation(outputLocation);
  OutputMeshComponent->SetWorldRotation(outputForwardVector.Rotation());

  GetWorld()->GetTimerManager().ClearTimer(CableSleepHandle);
  GetWorld()->GetTimerManager().SetTimer(
    CableSleepHandle,
    this,
    &AVCVCable::Sleep,
    1.2f, // seconds, apparently
    false
  );
  Wake();
}