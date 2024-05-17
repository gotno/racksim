#include "Utility/SvgWorker.h"

#include "ThirdParty/include/svgrender/svgrender.h"

#include "Engine/Texture2D.h"

FSvgWorker::FSvgWorker() {
}

FSvgWorker::~FSvgWorker() {
  Cleanup();
}

void FSvgWorker::MakeTexture(FString& inFilepath, UTexture2D* inTextureTarget, float inScale) {
  Cleanup();

  Filepath = inFilepath;
  TextureTarget = inTextureTarget;
  scale = inScale;
  Thread = FRunnableThread::Create(this, TEXT("SvgWorkerThread"));
}

void FSvgWorker::Cleanup() {
  bIsFinished = false;

  Filepath.Empty();

  if (TextureTarget) {
    TextureTarget = nullptr;
  }

  if (Thread) {
    Thread->WaitForCompletion();
    delete Thread;
    Thread = nullptr;
  }
}

bool FSvgWorker::Init() {
  if (Filepath.IsEmpty()) return false;
  if (!TextureTarget) return false;
  return true;
}

uint32 FSvgWorker::Run() {
  int width, height;

  unsigned char* rgba =
    renderSvgToPixelArray(TCHAR_TO_ANSI(*Filepath), width, height, scale);

  uint8* MipData = (uint8*)TextureTarget->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
  FMemory::Memcpy(MipData, rgba, width * height * sizeof(uint32));

  TextureTarget->GetPlatformData()->Mips[0].BulkData.Unlock();

  delete[] rgba;

  bIsFinished = true;

  return 0;
}