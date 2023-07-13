#include "IndicatorHUD.h"
#include "Engine/Texture2D.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"

AIndicatorHUD::AIndicatorHUD() {
  static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexture(TEXT("/Script/Engine.Texture2D'/Game/UI/indicator_20x.indicator_20x'"));
  if (CrosshairTexture.Object) {
    crosshair = CrosshairTexture.Object;
  }
}

void AIndicatorHUD::DrawHUD() {
  Super::DrawHUD();
  
  FVector2D center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
  FVector2d crosshairPosition(
      center.X - (crosshair->GetSurfaceWidth() * 0.5f),
      center.Y - (crosshair->GetSurfaceHeight() * 0.5f)
  );
  
  FCanvasTileItem tileItem(crosshairPosition, crosshair->GetResource(), FLinearColor::White);
  tileItem.BlendMode = SE_BLEND_Translucent;
  Canvas->DrawItem(tileItem);
}