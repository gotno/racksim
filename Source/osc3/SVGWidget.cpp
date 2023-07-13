#include "SVGWidget.h"

#include "Blueprint/WidgetTree.h"

#include "DefinitivePainter/Public/Canvas/DPCanvas.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "DefinitivePainter/Public/SVG/Importer/DPSVGImporter.h"
#include "DefinitivePainter/Public/Widgets/DPSVG.h"
#include "DefinitivePainter/Public/SVG/DPSVGAsset.h"

void USVGWidget::NativeOnInitialized() {
  Super::NativeOnInitialized();

  dpCanvas = WidgetTree->ConstructWidget<UDPCanvas>();
  dpCanvas->OneTimeRender = true;
  dpCanvas->ClearColor = FLinearColor(0, 0, 0, 1);

  svgWidget = NewObject<UDPSVG>(this, UDPSVG::StaticClass());
  svgAsset = NewObject<UDPSVGAsset>(this, UDPSVGAsset::StaticClass());
  svgWidget->SVG = svgAsset;

  canvasPanel = NewObject<UCanvasPanel>(this, UCanvasPanel::StaticClass());
  UCanvasPanelSlot* slot = canvasPanel->AddChildToCanvas(svgWidget);
  slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
  slot->SetOffsets(FMargin(0.f, 0.f));
  dpCanvas->AddChild(canvasPanel);

  WidgetTree->RootWidget = dpCanvas;
}

void USVGWidget::SetSVG(FString path) {
  FDPSVGImporter importer;
  importer.PerformImport(path, svgAsset);
  Super::NativeOnInitialized();
  svgAsset->GetImageSize();
}