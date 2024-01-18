#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SVGWidget.generated.h"

class UDPCanvas;
class UDPSVG;
class UDPSVGAsset;
class UTexture2D;

UCLASS()
class OSC3_API USVGWidget : public UUserWidget {
	GENERATED_BODY()
	
protected:
  virtual void NativeOnInitialized() override;

public:
  void SetSVG(UDPSVGAsset* SvgAsset);
  UTexture2D* GetTexture();

private:
  UDPSVG* dpSvgWidget;
  UDPCanvas* dpCanvas = nullptr;
};
