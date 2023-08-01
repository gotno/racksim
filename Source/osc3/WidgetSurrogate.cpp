#include "WidgetSurrogate.h"

#include "osc3GameModeBase.h"
#include "SVGWidget.h"

#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AWidgetSurrogate::AWidgetSurrogate() {
	PrimaryActorTick.bCanEverTick = true;

  SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SurrogateRoot"));
  RootComponent = SceneComponent;

  static ConstructorHelpers::FObjectFinder<UMaterial> WidgetMaterial(TEXT("/Script/Engine.Material'/Game/materials/SVGMaterial.SVGMaterial'"));

  if (WidgetMaterial.Object) {
    WidgetMaterialInterface = Cast<UMaterial>(WidgetMaterial.Object);
  }
  
  WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SurrogateWidgetComponent"));
  WidgetComponent->SetupAttachment(RootComponent);
  WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
  WidgetComponent->SetWidgetClass(USVGWidget::StaticClass());
}

void AWidgetSurrogate::BeginPlay() {
	Super::BeginPlay();

  if (WidgetMaterialInterface) {
    WidgetComponent->SetMaterial(0, WidgetMaterialInterface);
  }

  gameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AWidgetSurrogate::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (texture) return;

  texture = Cast<USVGWidget>(WidgetComponent->GetWidget())->GetTexture();
  if (texture) gameMode->RegisterTexture(svgFilepath, texture);
}

void AWidgetSurrogate::SetSVG(UDPSVGAsset* svgAsset, Vec2 size, FString filepath) {
  USVGWidget* widget = Cast<USVGWidget>(WidgetComponent->GetWidget());
  widget->SetSVG(svgAsset);
  svgFilepath = filepath;

  WidgetComponent->SetDrawSize(FVector2D(size.x, size.y) * drawSizeScale);
}