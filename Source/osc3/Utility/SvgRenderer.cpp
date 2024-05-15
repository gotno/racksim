#include "Utility/SvgRenderer.h"

#include "Utility/SvgWorker.h"

#include "Engine/Texture2D.h"

void USvgRenderer::RenderTextureAsync(const FString& Filepath) {
  Worker = new FSvgWorker(Filepath);

  GetWorld()->GetTimerManager().SetTimer(
    hFinished,
    this,
    &USvgRenderer::CheckFinished,
    0.02f, // 20ms
    true // loop
  );
}

void USvgRenderer::CheckFinished() {
  if (Worker->width == -1 || Worker->height == -1) return;

  GetWorld()->GetTimerManager().ClearTimer(hFinished);
  MakeTexture();
}

void USvgRenderer::MakeTexture() {
  int& width = Worker->width;
  int& height = Worker->height;
  FString& Filepath = Worker->Filepath;

  UTexture2D* texture = UTexture2D::CreateTransient(width, height, PF_R8G8B8A8);
  if (!texture) {
    UE_LOG(LogTemp, Warning, TEXT("SvgRenderer unable to create texture"));
    return;
  }
  
  uint8* MipData = (uint8*)texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
  FMemory::Memcpy(MipData, Worker->rgba, width * height * sizeof(uint32));
  
  texture->GetPlatformData()->Mips[0].BulkData.Unlock();
  texture->UpdateResource();

  OnTextureRenderedDelegate.Broadcast(Filepath, texture);
  delete Worker;
}