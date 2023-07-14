#include "SVGWidget.h"

#include "Blueprint/WidgetTree.h"

#include "DefinitivePainter/Public/Canvas/DPCanvas.h"
#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"
#include "DefinitivePainter/Public/Widgets/DPSVG.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"

#include "DefinitivePainter/Public/SVG/Tree/DPSVGRoot.h"
#include "DefinitivePainter/Public/SVG/Tree/DPSVGElement.h"
#include "DefinitivePainter/Public/SVG/Tree/DPSVGPaint.h"

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
  svgAsset = NewObject<UDPSVGAsset>(this, UDPSVGAsset::StaticClass());
  svgWidget->SVG = svgAsset;

  dpCanvas->AddChild(svgWidget);

  WidgetTree->RootWidget = dpCanvas;
}

FLinearColor USVGWidget::SetSVG(FString path) {
  FDPSVGImporter importer;
  importer.PerformImport(path, svgAsset);
  Super::NativeOnInitialized();

  return svgAsset->GetRootElement()->Paint.FillColor;
}