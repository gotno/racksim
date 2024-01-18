#include "SVG/WidgetSurrogate.h"

#include "osc3GameModeBase.h"
#include "SVG/SVGWidget.h"

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

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
}

void AWidgetSurrogate::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
  
  if (Texture) return;

  Texture = Cast<USVGWidget>(WidgetComponent->GetWidget())->GetTexture();
  if (Texture) GameMode->RegisterTexture(SvgFilepath, Texture);
}

void AWidgetSurrogate::SetSVG(UDPSVGAsset* SvgAsset, Vec2 Size, FString Filepath) {
  USVGWidget* widget = Cast<USVGWidget>(WidgetComponent->GetWidget());
  widget->SetSVG(SvgAsset);
  SvgFilepath = Filepath;

  WidgetComponent->SetDrawSize(FVector2D(Size.x, Size.y) * DrawSizeScale);
}