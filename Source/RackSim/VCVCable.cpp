#include "VCVCable.h"

#include "osc3GameModeBase.h"
#include "ModuleComponents/VCVPort.h"
#include "CableEnd.h"

#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AVCVCable::AVCVCable() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene"));
  SetRootComponent(RootSceneComponent);

  static ConstructorHelpers::FObjectFinder<UNiagaraSystem> NiagaraRef(TEXT("/Script/Niagara.NiagaraSystem'/Game/fx/BeamCable.BeamCable'"));
  if (NiagaraRef.Object) CableFXSystem = Cast<UNiagaraSystem>(NiagaraRef.Object);
}

void AVCVCable::BeginPlay() {
	Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

  CycleColor();

  if (CableFXSystem) {
    CableFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
      CableFXSystem,
      RootSceneComponent,
      NAME_None,
      FVector(0.f),
      FRotator(0.f),
      EAttachLocation::KeepRelativeOffset,
      true
    );
    // TODO: check for GameUserSettings
    Tension = DEFAULT_CABLE_TENSION;
    SetOpacity(DEFAULT_CABLE_OPACITY);
    SetColor(CableColor);
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

    CableFXComponent->AttachToComponent(
      CableEndA->GetMesh(),
      FAttachmentTransformRules::SnapToTargetNotIncludingScale,
      TEXT("wire")
    );
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

void AVCVCable::RecalculatePosition() {
  if (!CableFXComponent) return;

  if (FMath::IsNearlyZero(Tension)) {
    CableFXComponent->SetVectorParameter(FName("start_tangent"), -CableEndA->GetActorForwardVector());
    CableFXComponent->SetVectorParameter(FName("end_tangent"), -CableEndB->GetActorForwardVector());
    CableFXComponent->SetVectorParameter(FName("cable_end"), CableEndB->GetMesh()->GetSocketLocation(TEXT("wire")));
  } else {
    FVector socketLocationA = CableEndA->GetMesh()->GetSocketLocation(TEXT("wire"));
    FVector socketLocationB = CableEndB->GetMesh()->GetSocketLocation(TEXT("wire"));
    FVector abLookAtVector =
      UKismetMathLibrary::FindLookAtRotation(
        socketLocationA,
        socketLocationB
      ).Vector();
    CableFXComponent->SetVectorParameter(
      FName("start_tangent"),
      FMath::Lerp(-CableEndA->GetActorForwardVector(), abLookAtVector, Tension)
    );
    CableFXComponent->SetVectorParameter(
      FName("end_tangent"),
      FMath::Lerp(-CableEndB->GetActorForwardVector(), -abLookAtVector, Tension)
    );
    CableFXComponent->SetVectorParameter(FName("cable_end"), socketLocationB);
  }
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

    GameMode->RegisterCableConnect(inputPort, outputPort, CableColor);
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

void AVCVCable::SetTension(float inTension) {
  Tension = inTension;
  RecalculatePosition();
}

void AVCVCable::SetOpacity(float Opacity) {
  if (!CableFXComponent) return;

  CableFXComponent->SetFloatParameter(FName("alpha"), Opacity);
}

void AVCVCable::SetColor(FColor Color) {
  CableColor = Color;

  if (CableFXComponent)
    CableFXComponent->SetColorParameter(FName("color"), CableColor);
  if (CableEndA)
    CableEndA->SetColor(CableColor);
  if (CableEndB)
    CableEndB->SetColor(CableColor);
}

void AVCVCable::CycleColor() {
  CycleColor(CableColorCycleDirection);
}

void AVCVCable::CycleColor(int Direction) {
  CurrentCableColorIndex = CurrentCableColorIndex + Direction;
  if (CurrentCableColorIndex > CableColors.Num() - 1) {
    CurrentCableColorIndex = 0;
  } else if (CurrentCableColorIndex < 0) {
    CurrentCableColorIndex = CableColors.Num() - 1;
  }

  SetColor(CableColors[CurrentCableColorIndex]);
}
