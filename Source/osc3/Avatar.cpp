#include "Avatar.h"

#include "osc3.h"
#include "VCVParam.h"
#include "VCVPort.h"
#include "VCVCable.h"
#include "osc3GameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

AAvatar::AAvatar() {
	PrimaryActorTick.bCanEverTick = true;
  SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("root")));
  
  cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("cameara"));
  cameraComponent->SetupAttachment(GetRootComponent());
  cameraComponent->bUsePawnControlRotation = true;

  movementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("movement"));
  movementComponent->MaxSpeed = 50.f;
}

void AAvatar::BeginPlay() {
	Super::BeginPlay();
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AAvatar::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  sweep();
  drawIndicator();

  if (controlledParam) {
    float moveMouseX, moveMouseY;
    Cast<APlayerController>(Controller)->GetMousePosition(moveMouseX, moveMouseY);

    // UE_LOG(LogTemp, Warning, TEXT("controlling param %s, y distance: %f"), *controlledParam->GetActorNameOrLabel(), clickMouseY - moveMouseY);
    controlledParam->alter(clickMouseY - moveMouseY);
  }
  
  // this is terrible :D
  if (controlledCable) {
    // UE_LOG(LogTemp, Warning, TEXT("controlling cable %s"), *controlledCable->GetName());
    if (hasHit) {
      AVCVPort* port = Cast<AVCVPort>(hitActor);
      if (port && port->canConnect(controlledCable->getHangingType())) {
        FVector location, direction;
        gameMode->GetPortInfo(port->getIdentity(), location, direction);
        controlledCable->setHangingLocation(location, direction);
      } else {
        controlledCable->setHangingLocation(
          GetActorLocation() + cameraComponent->GetForwardVector() * 10.f,
          -cameraComponent->GetForwardVector()
        );
      }
    } else {
      controlledCable->disconnectFrom(controlledCable->getHangingType());
      controlledCable->setHangingLocation(
        GetActorLocation() + cameraComponent->GetForwardVector() * 10.f,
        -cameraComponent->GetForwardVector()
      );
    }
  }
}

void AAvatar::sweep() {
  FVector start = GetActorLocation();
  FVector end = start + cameraComponent->GetForwardVector() * interactDistance;

  hasHit = GetWorld()->LineTraceSingleByChannel(
    hitResult,
    start,
    end,
    PARAM_TRACE
  );
  
  if (!hasHit) {
    hovering = false;
    hitActor = nullptr;
    return;
  }

  hitActor = hitResult.GetActor();
  if (Cast<AVCVParam>(hitActor) || Cast<AVCVPort>(hitActor)) {
    hovering = true;
  }
}

// get the position+rotation for spawning a WidgetSurrogate in the camera's view
// so it is invisible (facing away) but renders immediately
void AAvatar::GetRenderablePosition(FVector& location, FRotator& rotation) {
  location = GetActorLocation() + cameraComponent->GetForwardVector() * 10.f;
  rotation = cameraComponent->GetForwardVector().Rotation();
}

void AAvatar::drawIndicator() {
  FTransform transform;
  transform.SetIdentity();
  FVector translation = GetActorLocation() + cameraComponent->GetForwardVector() * 2.f;
  transform.SetTranslation(translation);
  transform.SetRotation(cameraComponent->GetForwardVector().Rotation().Quaternion());

  DrawDebugCircle(
    GetWorld(),
    transform.ToMatrixNoScale(),
    0.02f,
    64,
    hovering ? indicatorColorHover : indicatorColor,
    false, -1.f, 0,
    0.005f
  );
}

void AAvatar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

  PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AAvatar::moveForward);
  PlayerInputComponent->BindAxis(TEXT("Right"), this, &AAvatar::moveRight);
  PlayerInputComponent->BindAxis(TEXT("Up"), this, &AAvatar::moveUp);

  PlayerInputComponent->BindAxis("LookUp", this, &AAvatar::AddControllerPitchInput);
  PlayerInputComponent->BindAxis("TurnRight", this, &AAvatar::AddControllerYawInput);
  
  PlayerInputComponent->BindAction(TEXT("LeftClick"), IE_Pressed, this, &AAvatar::click);
  PlayerInputComponent->BindAction(TEXT("LeftClick"), IE_Released, this, &AAvatar::release);
}

void AAvatar::moveForward(float axisValue) {
  if (Controller && axisValue != 0.f) {
    FRotator yawRotation(0, Controller->GetControlRotation().Yaw, 0);
    FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);

    AddMovementInput(direction, axisValue);
  }
}

void AAvatar::moveRight(float axisValue) {
  if (Controller && axisValue != 0.f) {
    FRotator yawRotation(0, Controller->GetControlRotation().Yaw, 0);
    FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(direction, axisValue);
  }
}

void AAvatar::moveUp(float axisValue) {
  AddMovementInput(GetActorUpVector(), axisValue);
}

void AAvatar::click() {
  if (hasHit) {
    Cast<APlayerController>(Controller)->GetMousePosition(clickMouseX, clickMouseY);
    
    if (Cast<AVCVParam>(hitActor)) {
      controlledParam = Cast<AVCVParam>(hitActor);
      controlledParam->engage();
    }
    
    if (Cast<AVCVPort>(hitActor)) {
      AVCVPort* clickedPort = Cast<AVCVPort>(hitActor);
      int64_t cableId = -1;

      if (clickedPort->getCableId(cableId)) {
        controlledCable = gameMode->DetachCable(cableId, clickedPort->getIdentity());
        UE_LOG(LogTemp, Warning, TEXT("got cable %lld"), cableId);
      } else {
        UE_LOG(LogTemp, Warning, TEXT("spawning new cable"));
        VCVCable cable(cableId);
        cable.setIdentity(clickedPort->getIdentity());
        controlledCable = gameMode->SpawnCable(cable);
      }
    }
  }
}

void AAvatar::release() {
  if (controlledParam) {
    controlledParam->release();
    controlledParam = nullptr;
    clickMouseX = 0.f;
    clickMouseY = 0.f;
  }

  if (controlledCable) {
    if (hasHit) {
      AVCVPort* port = Cast<AVCVPort>(hitActor);
      if (port && port->canConnect(controlledCable->getHangingType())) {
        gameMode->AttachCable(controlledCable->getId(), port->getIdentity());
      } else {
        gameMode->DestroyCable(controlledCable->getId());
      }
    } else {
      gameMode->DestroyCable(controlledCable->getId());
    }
    controlledCable = nullptr;
  }
}

void AAvatar::AddControllerPitchInput(float axisValue) {
  if (controlledParam) return;
  
  Super::AddControllerPitchInput(axisValue);
}

void AAvatar::AddControllerYawInput(float axisValue) {
  if (controlledParam) return;
  
  Super::AddControllerYawInput(axisValue);
}