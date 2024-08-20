#include "Player/VRMotionController.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "Utility/GrabbableActor.h"
#include "Player/VRAvatar.h"
#include "VCVData/VCV.h"
#include "VCVModule.h"
#include "VCVCable.h"
#include "CableEnd.h"
#include "ModuleComponents/VCVDisplay.h"
#include "ModuleComponents/VCVParam.h"
#include "ModuleComponents/VCVKnob.h"
#include "ModuleComponents/VCVSlider.h"
#include "ModuleComponents/VCVPort.h"
#include "UI/Tooltip.h"
#include "Utility/GrabbableActor.h"

#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/PointLightComponent.h"
#include "DrawDebugHelpers.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Kismet/GameplayStatics.h"

AVRMotionController::AVRMotionController() {
  PrimaryActorTick.bCanEverTick = true;

  MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
  SetRootComponent(MotionController);

  ControllerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Controller Mesh"));
  ControllerMesh->SetupAttachment(GetRootComponent());
  InteractIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Interact Indicator Mesh"));

  GrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("GrabSphere"));
  GrabSphere->InitSphereRadius(GrabSphereRadius);
  GrabSphere->SetupAttachment(GetRootComponent());

  Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
  Light->SetupAttachment(GetRootComponent());

  TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Tooltip"));
  TooltipWidgetComponent->SetupAttachment(GetRootComponent());

  WidgetInteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComponent"));
  WidgetInteractionComponent->SetupAttachment(GetRootComponent());
  WidgetInteractionComponent->VirtualUserIndex = 0;
  WidgetInteractionComponent->TraceChannel = WIDGET_TRACE;

  WidgetInteractionComponent->bShowDebug = false;
  WidgetInteractionComponent->InteractionDistance = 60.f;

  PointerFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PointerFXComponent"));
  PointerFXComponent->SetupAttachment(GetRootComponent());
}

void AVRMotionController::BeginPlay() {
  Super::BeginPlay();

  Avatar = Cast<AVRAvatar>(GetOwner());
  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  PlayerController = UGameplayStatics::GetPlayerController(this, 0);

  TooltipWidgetComponent->SetWorldScale3D(FVector(0.04f, 0.04f, 0.04f));
  TooltipWidgetComponent->SetWorldRotation(
    Avatar->GetLookAtCameraRotation(TooltipWidgetComponent->GetComponentLocation())
  );
  SetTooltipVisibility(false);
  TooltipWidget = Cast<UTooltip>(TooltipWidgetComponent->GetUserWidgetObject());

  OnParamTargetedDelegate.AddUObject(this, &AVRMotionController::HandleOwnTargetings);
  OnOriginPortTargetedDelegate.AddUObject(this, &AVRMotionController::HandleOwnTargetings);
  OnCableTargetedDelegate.AddUObject(this, &AVRMotionController::HandleOwnTargetings);
  OnCableHeldDelegate.AddUObject(this, &AVRMotionController::HandleOwnTargetings);
}

void AVRMotionController::HandleOwnTargetings(AActor* TargetedActor, EControllerHand Hand) {
  SetTooltipVisibility(!!TargetedActor);
  if (TargetedActor) HapticBump();
}

void AVRMotionController::SetTrackingSource(EControllerHand Hand) {
  MotionController->SetTrackingSource(Hand);

  HandName = Hand == EControllerHand::Left ? "left" : "right";
  WidgetInteractionComponent->PointerIndex = Hand == EControllerHand::Left ? 1 : 0; 
}

void AVRMotionController::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  SetActorHiddenInGame(!MotionController->IsTracked());

  WidgetInteractionTick();

  GrabbableTargetTick();
  // grab sphere
  // DrawDebugSphere(
  //   GetWorld(),
  //   GrabSphere->GetComponentLocation(),
  //   GrabSphere->GetUnscaledSphereRadius(),
  //   16,
  //   FColor::Blue
  // );

  InteractTraceStart = MotionController->GetComponentLocation();
  InteractTraceEnd = InteractTraceStart + MotionController->GetForwardVector() * InteractTraceReach;
  // interact trace
  // DrawDebugLine(GetWorld(), InteractTraceStart, InteractTraceEnd, FColor::Purple);

  ParamTargetTick();
  PortTargetTick();
  CableTargetTick();

  // SetIndicatorPosition();
  FVector indicatorLocation;
  FRotator indicatorRotation = MotionController->GetComponentRotation();
  if (TargetedParam) {
    float indicatorDistance = 0.5f + 0.2f; // interactor radius + a small space
    indicatorDistance += Cast<AVCVParam>(TargetedParam)->GetInteractHeight();
    if (Cast<AVCVKnob>(TargetedParam)) {
      indicatorLocation =
        TargetedParam->GetActorLocation() - TargetedParam->GetActorForwardVector() * indicatorDistance;
      indicatorRotation = TargetedParam->GetActorRotation();
    } else if (Cast<AVCVSlider>(TargetedParam)) {
      AVCVSlider* slider = Cast<AVCVSlider>(TargetedParam);
      indicatorLocation =
        slider->GetHandleLocation() - slider->GetActorForwardVector() * indicatorDistance;
    } else {
      indicatorLocation =
        TargetedParam->GetActorLocation() - TargetedParam->GetActorForwardVector() * indicatorDistance;
    }
  // } else if (!bIsPortInteracting && TargetedOriginPort && !Cast<AVCVPort>(TargetedOriginPort)->HasConnections()) {
  //   indicatorLocation =
  //     TargetedOriginPort->GetActorLocation() - TargetedOriginPort->GetActorForwardVector() * 1.f;
  } else {
    indicatorLocation =
      InteractTraceStart + MotionController->GetForwardVector() * InteractTraceReach * 0.5f;
  }
  InteractIndicatorMesh->SetWorldLocation(indicatorLocation);
  InteractIndicatorMesh->SetWorldRotation(indicatorRotation);

  if (TooltipWidgetComponent->IsVisible())
    TooltipWidgetComponent->SetWorldRotation(
      Avatar->GetLookAtCameraRotation(TooltipWidgetComponent->GetComponentLocation())
    );
}

void AVRMotionController::WidgetInteractionTick() {
  if (WidgetInteractionComponent->IsOverHitTestVisibleWidget() && !bIsWidgetInteracting) {
    bIsWidgetInteracting = true;
    Avatar->SetControllerWidgetInteracting(
      MotionController->GetTrackingSource(),
      bIsWidgetInteracting
    );
  } else if (!WidgetInteractionComponent->IsOverHitTestVisibleWidget() && bIsWidgetInteracting) {
    bIsWidgetInteracting = false;
    Avatar->SetControllerWidgetInteracting(
      MotionController->GetTrackingSource(),
      bIsWidgetInteracting
    );
  }

  if (WidgetInteractionComponent->IsOverHitTestVisibleWidget()) {
    FHitResult widgetHit = WidgetInteractionComponent->GetLastHitResult();

    PointerFXComponent->SetWorldLocation(InteractTraceEnd);
    PointerFXComponent->SetVectorParameter(FName("pointer_end"), widgetHit.Location);
    PointerFXComponent->SetVisibility(true);
  } else {
    PointerFXComponent->SetVisibility(false);
  }
}

void AVRMotionController::GrabbableTargetTick() {
  if (bIsGrabbing || bIsWorldInteracting) return;

  TSet<AActor*> overlappingActors;
  GrabSphere->GetOverlappingActors(overlappingActors, AGrabbableActor::StaticClass());

  FVector grabSphereLocation = GrabSphere->GetComponentLocation();

  double shortestDistance{9999999};
  AActor* closestGrabbable{nullptr};

  for (AActor* actor : overlappingActors) {
    double thisDistance = FVector::Dist(grabSphereLocation, actor->GetActorLocation());
    if (thisDistance < shortestDistance) {
      closestGrabbable = actor;
      shortestDistance = thisDistance;
    }
  }

  if (closestGrabbable != TargetedGrabbable) {
    TargetedGrabbable = closestGrabbable;

    OnGrabbableTargetedDelegate.Broadcast(
      TargetedGrabbable,
      MotionController->GetTrackingSource()
    );
  }
}

void AVRMotionController::NullifyInteractionTargets(AActor* Except) {
  if (TargetedParam && Except != TargetedParam) {
    TargetedParam = nullptr;
    OnParamTargetedDelegate.Broadcast(
      TargetedParam,
      MotionController->GetTrackingSource()
    );
  }

  if (TargetedOriginPort && Except != TargetedOriginPort) {
    TargetedOriginPort = nullptr;
    OnOriginPortTargetedDelegate.Broadcast(
      TargetedOriginPort,
      MotionController->GetTrackingSource()
    );
  }

  if (TargetedCableEnd && Except != TargetedCableEnd) {
    TargetedCableEnd = nullptr;
    OnCableTargetedDelegate.Broadcast(
      TargetedCableEnd,
      MotionController->GetTrackingSource()
    );
  }
}

void AVRMotionController::ParamTargetTick() {
  if (ControllerIsBusy()) return;

  FHitResult hit;
  FCollisionObjectQueryParams queryParams;
  queryParams.AddObjectTypesToQuery(PARAM_OBJECT);

  GetWorld()->LineTraceSingleByObjectType(hit, InteractTraceStart, InteractTraceEnd, queryParams);

  if (hit.GetActor() != TargetedParam) {
    TargetedParam = hit.GetActor();

    NullifyInteractionTargets(TargetedParam);
    OnParamTargetedDelegate.Broadcast(
      TargetedParam,
      MotionController->GetTrackingSource()
    );
  }
}

void AVRMotionController::PortTargetTick() {
  if (ControllerIsBusy()) return;

  FHitResult hit;
  FCollisionObjectQueryParams queryParams;
  queryParams.AddObjectTypesToQuery(PORT_OBJECT);

  AActor* hitPort{nullptr};
  if(GetWorld()->LineTraceSingleByObjectType(hit, InteractTraceStart, InteractTraceEnd, queryParams)) {
    if (!Cast<AVCVPort>(hit.GetActor())->HasConnections())
     hitPort = hit.GetActor();
  }

  if (hitPort != TargetedOriginPort) {
    TargetedOriginPort = hitPort;

    NullifyInteractionTargets(TargetedOriginPort);
    OnOriginPortTargetedDelegate.Broadcast(
      TargetedOriginPort,
      MotionController->GetTrackingSource()
    );
  }
}

void AVRMotionController::CableTargetTick() {
  if (ControllerIsBusy()) return;

  FHitResult hit;
  FCollisionObjectQueryParams queryParams;
  queryParams.AddObjectTypesToQuery(CABLE_END_OBJECT);

  ACableEnd* hitCableEnd{nullptr};
  if (GetWorld()->LineTraceSingleByObjectType(hit, InteractTraceStart, InteractTraceEnd, queryParams)) {
    hitCableEnd = Cast<ACableEnd>(hit.GetActor());
    if (hitCableEnd->IsConnected())
      hitCableEnd = hitCableEnd->GetConnectedPort()->GetTopCableEnd();

    if (!OnCableTargetedDelegate.IsBoundToObject(hitCableEnd))
      CableTargetedDelegates.Push(
        OnCableTargetedDelegate.AddUObject(hitCableEnd, &ACableEnd::HandleCableTargeted)
      );
    if (!OnCableHeldDelegate.IsBoundToObject(hitCableEnd))
      CableHeldDelegates.Push(
        OnCableHeldDelegate.AddUObject(hitCableEnd, &ACableEnd::HandleCableHeld)
      );
  }

  if (hitCableEnd != TargetedCableEnd) {
    TargetedCableEnd = hitCableEnd;

    NullifyInteractionTargets(TargetedCableEnd);
    OnCableTargetedDelegate.Broadcast(
      TargetedCableEnd,
      MotionController->GetTrackingSource()
    );
  }
}

void AVRMotionController::HapticBump() {
  PlayerController->PlayHapticEffect(HapticEffects.Bump, MotionController->GetTrackingSource());
}

void AVRMotionController::HapticThump() {
  PlayerController->PlayHapticEffect(HapticEffects.Thump, MotionController->GetTrackingSource());
}

void AVRMotionController::RefreshTooltip() {
  if (TargetedParam) {
    FString label, displayValue;
    Cast<AVCVParam>(TargetedParam)->GetTooltipText(label, displayValue);

    if (displayValue.IsEmpty()) {
      TooltipWidget->SetText(label);
    } else {
      TooltipWidget->SetText(label, displayValue, true);
    }

    return;
  }

  if (TargetedOriginPort) {
    FString name, description;
    Cast<AVCVPort>(TargetedOriginPort)->GetTooltipText(name, description);

    if (description.IsEmpty()) {
      TooltipWidget->SetText(name);
    } else {
      TooltipWidget->SetText(name, description, false, true);
    }

    return;
  }

  AVCVPort* thisEndPort{nullptr};
  AVCVPort* otherEndPort{nullptr};

  if (TargetedCableEnd) {
    ACableEnd* end = Cast<ACableEnd>(TargetedCableEnd);
    thisEndPort = end->GetPort();
    otherEndPort = end->Cable->GetOtherPort(end);
  }

  if (HeldCableEnd) {
    if (TargetedDestinationPort)
      thisEndPort = Cast<AVCVPort>(TargetedDestinationPort);
    otherEndPort = HeldCableEnd->Cable->GetOtherPort(HeldCableEnd);
  }

  if (!thisEndPort && !otherEndPort) {
    SetTooltipVisibility(false);
    return;
  }

  FString fromName, fromDescription, toName, toDescription;
  if (thisEndPort) thisEndPort->GetTooltipText(toName, toDescription);
  if (otherEndPort) otherEndPort->GetTooltipText(fromName, fromDescription);

  if (thisEndPort && otherEndPort) {
    TooltipWidget->SetText(
      FString("from: ").Append(fromName),
      FString("to: ").Append(toName)
    );
  } else if (thisEndPort) {
    if (toDescription.IsEmpty()) {
      TooltipWidget->SetText(toName);
    } else {
      TooltipWidget->SetText(toName, toDescription, false, true);
    }
  } else {
    TooltipWidget->SetText(
      FString("from: ").Append(fromName)
    );
  }
}

void AVRMotionController::SetWorldInteract(bool bActive) {
  bIsWorldInteracting = bActive;
}

void AVRMotionController::StartParamInteract() {
  bIsParamInteracting = true;
}

void AVRMotionController::EndParamInteract() {
  bIsParamInteracting = false;
}

void AVRMotionController::StartPortInteract() {
  bIsPortInteracting = true;

  if (TargetedCableEnd) {
    HeldCableEnd = Cast<ACableEnd>(TargetedCableEnd);
    if (HeldCableEnd->IsConnected()) HeldCableEnd->Disconnect();
  } else if (TargetedOriginPort) {
    AVCVPort* port = Cast<AVCVPort>(TargetedOriginPort);
    AVCVCable* newCable = GameMode->SpawnCable(port);
    HeldCableEnd = newCable->GetOtherEnd(port);
    OnCableHeldDelegate.AddUObject(HeldCableEnd, &ACableEnd::HandleCableHeld);
    TargetedOriginPort = nullptr;
    RefreshTooltip();
  }

  HeldCableEnd->OnDestinationPortTargetedDelegate.AddUObject(
    this,
    &AVRMotionController::HandleDestinationPortTargeted
  );

  OnCableHeldDelegate.Broadcast(
    HeldCableEnd,
    MotionController->GetTrackingSource()
  );
}

void AVRMotionController::HandleDestinationPortTargeted(AVCVPort* Port) {
  TargetedDestinationPort = Port;
  RefreshTooltip();
  if (Port && HeldCableEnd && Port->CanConnect(HeldCableEnd->GetType()))
    HapticBump();
}

void AVRMotionController::EndPortInteract(bool Connected) {
  OnCableHeldDelegate.Broadcast(
    nullptr,
    MotionController->GetTrackingSource()
  );
  HeldCableEnd = nullptr;

  for (FDelegateHandle& delegate : CableTargetedDelegates)
    OnCableTargetedDelegate.Remove(delegate);
  CableTargetedDelegates.Empty();
  for (FDelegateHandle& delegate : CableHeldDelegates)
    OnCableHeldDelegate.Remove(delegate);
  CableHeldDelegates.Empty();

  if (Connected) HapticBump();
  bIsPortInteracting = false;
}

void AVRMotionController::StartGrab() {
  bIsGrabbing = true;
  SetTooltipVisibility(false);
  HapticBump();

  // don't let param/port targets interfere with
  // trigger for snapping/context menu,
  // but allow holding cable end and grabbing at the same time
  if (TargetedOriginPort || TargetedParam)
    NullifyInteractionTargets(TargetedCableEnd);
}

void AVRMotionController::EndGrab() {
  bIsGrabbing = false;
}

void AVRMotionController::StartWidgetLeftClick() {
  WidgetInteractionComponent->PressPointerKey(EKeys::LeftMouseButton);

  // TODO: timer delay Xms, starts: timer looping Yms, runs: release, press
  // can we read a prop from the button to gate this behavior?
}

void AVRMotionController::WidgetScroll(float ScrollDelta) {
  WidgetInteractionComponent->ScrollWheel(ScrollDelta);
}

void AVRMotionController::EndWidgetLeftClick() {
  WidgetInteractionComponent->ReleasePointerKey(EKeys::LeftMouseButton);

  // TODO: kill timer
}

void AVRMotionController::SetTooltipVisibility(bool bVisible) {
  if (bVisible && !bTooltipEnabled) return;
  TooltipWidgetComponent->SetVisibility(bVisible, true);
  if (bVisible) RefreshTooltip();
}

void AVRMotionController::GetHeldCableEndInfo(FVector& Location, FVector& ForwardVector) {
  Location =
    MotionController->GetComponentLocation()
      + MotionController->GetForwardVector() * 3.f;
  ForwardVector = MotionController->GetForwardVector();
}

void AVRMotionController::SetLightHidden(bool bLightHidden) {
  Light->SetHiddenInGame(bLightHidden);
}

void AVRMotionController::SetTooltipHidden(bool bTooltipHidden) {
  TooltipWidgetComponent->SetHiddenInGame(bTooltipHidden);
}