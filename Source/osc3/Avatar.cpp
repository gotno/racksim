#include "Avatar.h"

#include "VCVParam.h"
#include "VCVPort.h"

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
}

void AAvatar::sweep() {
  FVector start = GetActorLocation();
  FVector end = start + cameraComponent->GetForwardVector() * interactDistance;

  hasHit = GetWorld()->LineTraceSingleByChannel(
    hitResult,
    start,
    end,
    ECC_Visibility
  );
  
  if (!hasHit) {
    hovering = false;
    return;
  }
  
  AActor* hitActor = hitResult.GetActor();
  if (Cast<AVCVParam>(hitActor) || Cast<AVCVPort>(hitActor)) {
    hovering = true;
  }
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
    AActor* hitActor = hitResult.GetActor();
    
    Cast<APlayerController>(Controller)->GetMousePosition(clickMouseX, clickMouseY);
    
    if (Cast<AVCVParam>(hitActor)) {
      controlledParam = Cast<AVCVParam>(hitActor);
      controlledParam->engage();
    } else if (Cast<AVCVPort>(hitActor)) {
      UE_LOG(LogTemp, Warning, TEXT("clicked port %s"), *hitActor->GetName());

      AVCVPort* clickedPort = Cast<AVCVPort>(hitActor);
      int64_t cableId = -1;

      if (clickedPort->getCableId(cableId)) {
        UE_LOG(LogTemp, Warning, TEXT("got cableId %lld"), cableId);
      } else {
        UE_LOG(LogTemp, Warning, TEXT("no cable"));
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
}

void AAvatar::AddControllerPitchInput(float axisValue) {
  if (controlledParam) return;
  
  Super::AddControllerPitchInput(axisValue);
}

void AAvatar::AddControllerYawInput(float axisValue) {
  if (controlledParam) return;
  
  Super::AddControllerYawInput(axisValue);
}