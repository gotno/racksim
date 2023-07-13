#include "VCVModule.h"
#include "VCVLight.h"
#include "VCVKnob.h"
#include "VCVButton.h"
#include "VCVSwitch.h"
#include "VCVSlider.h"
#include "VCVPort.h"
#include "VCVDisplay.h"
#include "osc3GameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AVCVModule::AVCVModule() {
	PrimaryActorTick.bCanEverTick = true;

  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  RootComponent = SceneComponent;

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  StaticMeshComponent->SetupAttachment(RootComponent);
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Script/Engine.Material'/Game/materials/module_body.module_body'"));
  
  if (Material.Object) {
    MaterialInterface = Cast<UMaterial>(Material.Object);
  }
}

void AVCVModule::BeginPlay() {
	Super::BeginPlay();

  if (MaterialInterface) {
    MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, MaterialInstance);
  }
  
  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AVCVModule::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AVCVModule::init(VCVModule vcv_module) {
  model = vcv_module; 
  StaticMeshComponent->SetWorldScale3D(FVector(1, model.box.size.x, model.box.size.y));
  spawnComponents();
  SetActorRotation(FRotator(-8.f, 0, 0));
}

void AVCVModule::GetPortInfo(PortIdentity identity, FVector& portLocation, FVector& portForwardVector) {
  AVCVPort* port;
  if (identity.type == PortType::Input) {
    port = InputActors[identity.portId];
  } else {
    port = OutputActors[identity.portId];
  }
  portLocation = port->GetActorLocation();
  portForwardVector = port->GetActorForwardVector();
}

void AVCVModule::AttachCable(const PortIdentity& identity, int64_t cableId) {
  if (identity.type == PortType::Input) {
    InputActors[identity.portId]->addCableId(cableId);
  } else {
    OutputActors[identity.portId]->addCableId(cableId);
  }
}

void AVCVModule::DetachCable(const PortIdentity& identity, int64_t cableId) {
  if (identity.type == PortType::Input) {
    InputActors[identity.portId]->removeCableId(cableId);
  } else {
    OutputActors[identity.portId]->removeCableId(cableId);
  }
}

void AVCVModule::UpdateLight(int32 lightId, FLinearColor color) {
  if (LightActors.Contains(lightId)) {
    LightActors[lightId]->SetEmissiveColor(color);
    LightActors[lightId]-> SetEmissiveIntensity(color.A);
  } else if (ParamLightActors.Contains(lightId)) {
    ParamLightActors[lightId]->SetEmissiveColor(color);
    ParamLightActors[lightId]-> SetEmissiveIntensity(color.A);
  }
}
void AVCVModule::registerParamLight(int64_t lightId, AVCVLight* lightActor) {
  ParamLightActors.Add(lightId, lightActor);
}

void AVCVModule::paramUpdated(int32 paramId, float value) {
  if (!gameMode) return;
  gameMode->SendParamUpdate(model.id, paramId, value);
}

void AVCVModule::spawnComponents() {
  FActorSpawnParameters spawnParams;
  spawnParams.Owner = this;

  for (TPair<int32, VCVParam>& param_kvp : model.Params) {
    VCVParam& param = param_kvp.Value;
    if (param.type == ParamType::Knob) {
      AVCVKnob* a_knob = GetWorld()->SpawnActor<AVCVKnob>(
        AVCVKnob::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
      a_knob->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
      a_knob->init(&param);
    } else if (param.type == ParamType::Slider) {
      AVCVSlider* a_slider = GetWorld()->SpawnActor<AVCVSlider>(
        AVCVSlider::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
      a_slider->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
      a_slider->init(&param);
    } else if (param.type == ParamType::Switch) {
      AVCVSwitch* a_switch = GetWorld()->SpawnActor<AVCVSwitch>(
        AVCVSwitch::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
      a_switch->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
      a_switch->init(&param);
    } else if (param.type == ParamType::Button) {
      AVCVButton* a_button = GetWorld()->SpawnActor<AVCVButton>(
        AVCVButton::StaticClass(),
        GetActorLocation() + param.box.location(),
        FRotator(0, 0, 0),
        spawnParams
      );
      a_button->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
      a_button->init(&param);
    }
  }

  for (TPair<int32, VCVPort>& port_kvp : model.Inputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->init(&port);
    InputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVPort>& port_kvp : model.Outputs) {
    VCVPort& port = port_kvp.Value;

    AVCVPort* a_port = GetWorld()->SpawnActor<AVCVPort>(
      AVCVPort::StaticClass(),
      GetActorLocation() + port.box.location() + FVector(0.01f, 0, 0),
      FRotator(0, 0, 0),
      spawnParams
    );
    a_port->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_port->init(&port);
    OutputActors.Add(port.id, a_port);
  }

  for (TPair<int32, VCVLight>& light_kvp : model.Lights) {
    VCVLight& light = light_kvp.Value;

    AVCVLight* a_light = GetWorld()->SpawnActor<AVCVLight>(
      AVCVLight::StaticClass(),
      GetActorLocation() + light.box.location(),
      FRotator(0, 0, 0)
    );
    a_light->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_light->init(&light);
    LightActors.Add(light.id, a_light);
  }

  for (VCVDisplay& display : model.Displays) {
    AVCVDisplay* a_display = GetWorld()->SpawnActor<AVCVDisplay>(
      AVCVDisplay::StaticClass(),
      GetActorLocation() + display.box.location(),
      FRotator(0, 0, 0)
    );
    a_display->AttachToComponent(StaticMeshComponent, FAttachmentTransformRules::KeepWorldTransform);
    a_display->init(&display);
  }
}