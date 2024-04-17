#include "VCVCable.h"

#include "osc3GameModeBase.h"
#include "ModuleComponents/VCVPort.h"
#include "CableEnd.h"

#include "CableComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

// #include "NiagaraFunctionLibrary.h"
// #include "NiagaraComponent.h"

AVCVCable::AVCVCable() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
  SetRootComponent(RootSceneComponent);

  static ConstructorHelpers::FObjectFinder<UMaterial> CableMaterial(CableMaterialReference);
  if (CableMaterial.Object) CableMaterialInterface = Cast<UMaterial>(CableMaterial.Object);

  // static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraRef(TEXT("/Script/Niagara.NiagaraSystem'/Game/NewNiagaraSystem.NewNiagaraSystem'"));
  // if (NiagaraRef.Object) NiaSys = Cast<UNiagaraSystem>(NiagaraRef.Object);
  
  CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("Cable Component"));
  CableComponent->EndLocation = FVector(0.f);
  CableComponent->CableWidth = 0.22f * RENDER_SCALE;
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

void AVCVCable::BeginPlay() {
	Super::BeginPlay();
  
  // if (NiaSys) {
  //   UNiagaraFunctionLibrary::SpawnSystemAttached(
  //     NiaSys,
  //     RootSceneComponent,
  //     NAME_None,
  //     FVector(0.f),
  //     FRotator(0.f),
  //     EAttachLocation::KeepRelativeOffset,
  //     true
  //   );
  // }

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

  CableColor = CableColors[CurrentCableColorIndex++];
  if (CurrentCableColorIndex > CableColors.Num() - 1) CurrentCableColorIndex = 0;

  if (CableMaterialInterface) {
    CableMaterialInstance = UMaterialInstanceDynamic::Create(CableMaterialInterface, this);
    CableComponent->SetMaterial(0, CableMaterialInstance);
    CableMaterialInstance->SetVectorParameterValue(FName("Color"), CableColor);
    CableMaterialInstance->SetScalarParameterValue(FName("Opacity"), CABLE_OPACITY);
  }

  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;
  if (!CableEndA) {
    CableEndA =
      GetWorld()->SpawnActor<ACableEnd>(
        ACableEnd::StaticClass(),
        FVector(0.f),
        FRotator(0.f),
        spawnParams
      );
    CableEndA->SetColor(CableColor);
    CableComponent->AttachToComponent(CableEndA->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("wire"));
  }
  if (!CableEndB) {
    CableEndB =
      GetWorld()->SpawnActor<ACableEnd>(
        ACableEnd::StaticClass(),
        FVector(0.f),
        FRotator(0.f),
        spawnParams
      );
    CableEndB->SetColor(CableColor);
    CableComponent->SetAttachEndToComponent(CableEndB->GetMesh(), TEXT("wire"));
  }
}

void AVCVCable::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);
  
  CableEndA->Disconnect();
  CableEndA->Destroy();
  CableEndB->Disconnect();
  CableEndB->Destroy();
}

void AVCVCable::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVCable::Sleep() {
  CableComponent->SetComponentTickEnabled(false);
}

void AVCVCable::Wake() {
  CableComponent->SetComponentTickEnabled(true);
}

void AVCVCable::Stir(float WakeSeconds) {
  float distanceBetweenEnds =
    FVector::Distance(
      CableEndA->GetActorLocation(),
      CableEndB->GetActorLocation()
    );
  CableComponent->CableLength = distanceBetweenEnds * 0.8f;

  // naive attempt to set cable force relative to average of cable end vectors
  // actually working kind of ok!
  CableComponent->CableForce =
    (CableEndA->GetActorForwardVector() + CableEndB->GetActorForwardVector())
      * 0.5f * -500.f;

  GetWorld()->GetTimerManager().ClearTimer(CableSleepHandle);
  GetWorld()->GetTimerManager().SetTimer(
    CableSleepHandle,
    this,
    &AVCVCable::Sleep,
    WakeSeconds,
    false // loop
  );
  Wake();
}

void AVCVCable::ConnectToPort(AVCVPort* Port) {
  if (CableEndA->IsConnected()) {
    CableEndB->Connect(Port);
  } else {
    CableEndA->Connect(Port);
  }

  HandleRegistration();
}

void AVCVCable::DisconnectFromPort(PortType Type) {
  if (CableEndA->GetType() == Type) {
    CableEndA->Disconnect();
  } else {
    CableEndB->Disconnect();
  }

  HandleRegistration();
}

AVCVPort* AVCVCable::GetOtherPort(ACableEnd* CableEnd) {
  return CableEnd == CableEndA
    ? CableEndB->GetPort()
    : CableEndA->GetPort();
}

ACableEnd* AVCVCable::GetOtherEnd(AVCVPort* Port) {
  return CableEndA->IsConnected() && CableEndA->GetPort() == Port
    ? CableEndB
    : CableEndA;
}

AVCVPort* AVCVCable::GetPort(PortType Type) {
  if (CableEndA->GetType() == Type) return CableEndA->GetPort();
  if (CableEndB->GetType() == Type) return CableEndB->GetPort();
  return nullptr;
}

void AVCVCable::Abandon() {
  if (!bLatched) GameMode->DestroyCableActor(this);
}

void AVCVCable::HandleRegistration() {
  if (IsComplete() && !IsRegistered()) {
    AVCVPort* inputPort;
    AVCVPort* outputPort;
    if (CableEndA->GetType() == PortType::Input) {
      inputPort = CableEndA->GetPort();
      outputPort = CableEndB->GetPort();
    } else {
      inputPort = CableEndB->GetPort();
      outputPort = CableEndA->GetPort();
    }

    GameMode->RegisterCableConnect(inputPort, outputPort);
    return;
  }

  if (IsRegistered() && !IsComplete()) {
    GameMode->RegisterCableDisconnect(this);
    return;
  }
}

bool AVCVCable::IsRegistered() {
  return Id != -1;
}

bool AVCVCable::IsComplete() {
  return CableEndA->IsConnected() && CableEndB->IsConnected();
}