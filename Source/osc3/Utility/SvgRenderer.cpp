#include "Utility/SvgRenderer.h"

#include "ThirdParty/include/svgrender/svgrender.h"

#include "Engine/Texture2D.h"

UTexture2D* USvgRenderer::GetTexture(const FString& Filepath) {
  int width, height;
  unsigned char* rgba =
    renderSvgToPixelArray(TCHAR_TO_ANSI(*Filepath), width, height, 3.f);

  UTexture2D* texture = UTexture2D::CreateTransient(width, height, PF_R8G8B8A8);
  if (!texture) {
    UE_LOG(LogTemp, Warning, TEXT("SvgRenderer unable to create texture"));
    return nullptr;
  }
  
  uint8* MipData = (uint8*)texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
  FMemory::Memcpy(MipData, rgba, width * height * sizeof(uint32));
  delete[] rgba;
  
  texture->GetPlatformData()->Mips[0].BulkData.Unlock();
  texture->UpdateResource();

  return texture;
}