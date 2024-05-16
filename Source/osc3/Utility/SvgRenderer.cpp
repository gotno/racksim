#include "Utility/SvgRenderer.h"

#include "osc3.h"
#include "Utility/SvgWorker.h"
#include "ThirdParty/include/svgrender/svgrender.h"

#include "Engine/Texture2D.h"

void USvgRenderer::BeginDestroy() {
  Super::BeginDestroy();

  if (Worker) {
    delete Worker;
    Worker = nullptr;
  }
}

void USvgRenderer::RenderTextureAsync(FString Filepath) {
  if (!Worker) Worker = new FSvgWorker();

  float scale = 1.5 * RENDER_SCALE;
  int width, height;

  getSvgSize(TCHAR_TO_ANSI(*Filepath), width, height, scale);

  TextureTarget = UTexture2D::CreateTransient(width, height, PF_R8G8B8A8);
  if (!TextureTarget) {
    UE_LOG(LogTemp, Warning, TEXT("SvgRenderer unable to create texture"));
    return;
  }

  Worker->MakeTexture(Filepath, TextureTarget, scale);

  GetWorld()->GetTimerManager().SetTimer(
    hFinished,
    this,
    &USvgRenderer::CheckFinished,
    0.02f, // 20ms
    true // loop
  );
}

void USvgRenderer::CheckFinished() {
  if (!Worker->bIsFinished) return;

  GetWorld()->GetTimerManager().ClearTimer(hFinished);

  OnTextureRenderedDelegate.Broadcast(Worker->Filepath, TextureTarget);
}