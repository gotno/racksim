#include "SVGWidget.h"

#include "Blueprint/WidgetTree.h"

#include "DefinitivePainter/Public/Canvas/DPCanvas.h"
#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"
#include "DefinitivePainter/Public/Widgets/DPSVG.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"

#include "DefinitivePainter/Public/SVG/Tree/DPSVGRoot.h"
#include "DefinitivePainter/Public/SVG/Tree/DPSVGElement.h"
#include "DefinitivePainter/Public/SVG/Tree/DPSVGPaint.h"

#include "osc3GameModeBase.h"
#include "Kismet/GameplayStatics.h"

void USVGWidget::NativeOnInitialized() {
  if (dpCanvas) return;
  Super::NativeOnInitialized();

  dpCanvas = WidgetTree->ConstructWidget<UDPCanvas>();

  // without this,
  // LogTemp: Error: Definitive Painter Context runtime error: Invalid window (BeginRendering)
  // Private/Context/Base/DPContextGPU.cpp:L153
  dpCanvas->EnableGPUAcceleration = false;

  dpCanvas->OneTimeRender = true;
  dpCanvas->ClearColor = FLinearColor(0, 0, 0, 1);

  svgWidget = NewObject<UDPSVG>(this, UDPSVG::StaticClass());

  dpCanvas->AddChild(svgWidget);

  WidgetTree->RootWidget = dpCanvas;
}

FLinearColor USVGWidget::SetSVG(FString path) {
  Aosc3GameModeBase* gm = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  svgWidget->SVG = gm->GetSVGAsset(path);

  Super::NativeOnInitialized();

  return svgWidget->SVG->GetRootElement()->Paint.FillColor;
}