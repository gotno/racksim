#include "CableEnd.h"

#include "VCVCable.h"
#include "ModuleComponents/VCVPort.h"

#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

ACableEnd::ACableEnd() {
  PrimaryActorTick.bCanEverTick = true;

  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
  SetRootComponent(SceneComponent);

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));

  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetCollisionObjectType(CABLE_END_OBJECT);
  StaticMeshComponent->SetupAttachment(GetRootComponent());

  Collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collider"));
  Collider->InitCapsuleSize(ColliderCapsuleRadius, ColliderCapsuleHalfHeight);
  Collider->SetWorldRotation(FRotator(90.f, 0.f, 0.f));
  Collider->AddLocalOffset(ColliderOffset);
  Collider->SetupAttachment(GetRootComponent());

  Collider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  Collider->SetCollisionResponseToChannel(PORT_OBJECT, ECollisionResponse::ECR_Overlap);

  static ConstructorHelpers::FObjectFinder<UStaticMesh> JackBody(JackMeshReference);
  if (JackBody.Object) {
    StaticMeshComponent->SetStaticMesh(JackBody.Object);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(BaseMaterialReference);
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }
}

void ACableEnd::BeginPlay() {
  Super::BeginPlay();

  Cable = Cast<AVCVCable>(GetOwner());

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
    RingMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(1, RingMaterialInstance);
  }

  Collider->OnComponentBeginOverlap.AddDynamic(this, &ACableEnd::HandleColliderOverlapStart);
  Collider->OnComponentEndOverlap.AddDynamic(this, &ACableEnd::HandleColliderOverlapEnd);
}

void ACableEnd::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // DrawDebugCapsule(
  //   GetWorld(),
  //   Collider->GetComponentLocation(),
  //   Collider->GetScaledCapsuleHalfHeight(),
  //   Collider->GetScaledCapsuleRadius(),
  //   Collider->GetComponentRotation().Quaternion(),
  //   FColor::White
  // );

  if (SnapToPort) {
    UpdatePosition();
  }
}

void ACableEnd::Connect(AVCVPort* Port) {
  ConnectedPort = Port;
  ConnectedPort->Connect(this);
  hPortTransformUpdated =
    ConnectedPort->GetRootComponent()->TransformUpdated.AddUObject(
      this,
      &ACableEnd::HandlePortTransformUpdated
    );
  SetSnapToPort(nullptr);
  RealignMesh();
  UpdatePosition();
  Cable->HandleRegistration();
}

void ACableEnd::HandleDisconnected() {
  ConnectedPort = nullptr;
  SetSnapToPort(nullptr);
  RealignMesh();
  Cable->HandleRegistration();
}

void ACableEnd::Disconnect() {
  if (ConnectedPort) {
    ConnectedPort->GetRootComponent()->TransformUpdated.Remove(
      hPortTransformUpdated
    );
    ConnectedPort->Disconnect(this);
  }
  HandleDisconnected();
}

bool ACableEnd::Drop() {
  bool connected{false};
  if (SnapToPort) {
    Connect(SnapToPort);
    connected = true;
  } else {
    Cable->Abandon();
  }
  OnDestinationPortTargetedDelegate.Clear();
  return connected;
}

AVCVPort* ACableEnd::GetConnectedPort() {
  return ConnectedPort;
}

AVCVPort* ACableEnd::GetPort() {
  if (ConnectedPort) return ConnectedPort;
  if (SnapToPort) return SnapToPort;
  return nullptr;
}

PortType ACableEnd::GetType() {
  AVCVPort* port;

  port = GetPort();
  if (port) return port->Type;

  port = Cable->GetOtherPort(this);
  if (port) {
    return port->Type == PortType::Input
      ? PortType::Output
      : PortType::Input;
  }

  return PortType::Any;
}

void ACableEnd::HandlePortTransformUpdated(
  USceneComponent* UpdatedComponent,
  EUpdateTransformFlags _UpdateTransformFlags,
  ETeleportType _Teleport
) {
  UpdatePosition();
}

void ACableEnd::UpdatePosition() {
  AVCVPort* port = GetPort();
  if (!port) return;

  if (port == ConnectedPort) {
    SetPosition(
      port->GetActorLocation(),
      port->GetActorForwardVector().Rotation()
    );
  }

  if (port == SnapToPort) {
    OffsetMeshFrom(SnapToPort);
  }
}

void ACableEnd::RealignMesh() {
  StaticMeshComponent->SetWorldLocation(GetActorLocation());
  StaticMeshComponent->SetWorldRotation(GetActorForwardVector().Rotation());
}

float ACableEnd::GetMeshOffset() {
  return 0.2f * AVCVCable::Scale;
}

void ACableEnd::OffsetMeshFrom(AActor* Actor) {
  FVector forwardVector = Actor->GetActorForwardVector();
  StaticMeshComponent->SetWorldLocation(
    Actor->GetActorLocation() - forwardVector * GetMeshOffset()
  );
  StaticMeshComponent->SetWorldRotation(forwardVector.Rotation());
}

void ACableEnd::GetPosition(FVector& Location, FRotator& Rotation) {
  Location = GetActorLocation();
  Rotation = GetActorRotation();
}

void ACableEnd::SetPosition(FVector Location, FRotator Rotation) {
  SetActorLocation(Location);
  SetActorRotation(Rotation);
}

void ACableEnd::HandleCableTargeted(AActor* CableEnd, EControllerHand Hand) {
  if (CableEnd == this && ConnectedPort) {
    OffsetMeshFrom(this);
  } else {
    RealignMesh();
  }
}

void ACableEnd::HandleCableHeld(AActor* CableEnd, EControllerHand Hand) {
  bool& held =
    Hand == EControllerHand::Left
      ? bHeldByLeftHand
      : bHeldByRightHand;
  held = CableEnd == this ? true : false;
  if (CableEnd == this) RealignMesh();
}

void ACableEnd::HandleColliderOverlapStart(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
  if (!IsHeld() || !Cast<AVCVPort>(OtherActor)->CanConnect(GetType())) return;
  SetSnapToPort(Cast<AVCVPort>(OtherActor));
}

void ACableEnd::HandleColliderOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
  if (!IsHeld()) return;

  TSet<AActor*> overlappingActors;
  Collider->GetOverlappingActors(overlappingActors, AVCVPort::StaticClass());
  
  TArray<AActor*> connectablePorts =
    overlappingActors.Array().FilterByPredicate([&](AActor* overlappedPort) {
      return Cast<AVCVPort>(overlappedPort)->CanConnect(GetType());
    });

  if (connectablePorts.IsEmpty()) {
    SetSnapToPort(nullptr);
    RealignMesh();
  } else if (connectablePorts.Num() == 1) { // snap to new overlapping port
    SetSnapToPort(Cast<AVCVPort>(connectablePorts[0]));
  } else { // find closest of n overlapping ports
    double shortestDistance{9999999};
    AActor* closestPort = nullptr;

    for (AActor* overlappedPort : connectablePorts) {
      double thisDistance =
        FVector::Dist(
          Collider->GetComponentLocation(),
          overlappedPort->GetActorLocation()
        );
      if (thisDistance < shortestDistance) {
        closestPort = overlappedPort;
        shortestDistance = thisDistance;
      }
    }

    SetSnapToPort(Cast<AVCVPort>(closestPort));
  }
}

void ACableEnd::SetColor(FColor Color) {
  BaseMaterialInstance->SetVectorParameterValue(FName("Color"), Color);
  RingMaterialInstance->SetVectorParameterValue(FName("Color"), Cable->IsLatched() ? FColor::Black : Color);
}

void ACableEnd::SetSnapToPort(AVCVPort* Port) {
  SnapToPort = Port;
  OnDestinationPortTargetedDelegate.Broadcast(Port);
}