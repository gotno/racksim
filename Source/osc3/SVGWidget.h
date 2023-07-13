#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SVGWidget.generated.h"

class UDPCanvas;
class UCanvasPanel;
class UDPSVG;
class UDPSVGAsset;

UCLASS()
class OSC3_API USVGWidget : public UUserWidget {
	GENERATED_BODY()
	
protected:
  virtual void NativeOnInitialized() override;

public:
  void SetSVG(FString path);

private:
  UDPCanvas* dpCanvas = nullptr;
  UCanvasPanel* canvasPanel;
  UDPSVG* svgWidget;
  UDPSVGAsset* svgAsset;
};
