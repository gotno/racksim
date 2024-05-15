#include "Utility/SvgWorker.h"

#include "osc3.h"
#include "ThirdParty/include/svgrender/svgrender.h"

FSvgWorker::FSvgWorker(FString inFilepath) {
  Filepath = inFilepath;
  Thread = FRunnableThread::Create(this, TEXT("SvgWorkerThread"));
}

FSvgWorker::~FSvgWorker() {
  delete[] rgba;
  delete Thread;
  Thread = nullptr;
}

bool FSvgWorker::Init() {
  return true;
}

uint32 FSvgWorker::Run() {
  int outWidth, outHeight;

  float scale = 1.5f * RENDER_SCALE;

  rgba =
    renderSvgToPixelArray(TCHAR_TO_ANSI(*Filepath), outWidth, outHeight, scale);
  
  width = outWidth;
  height = outHeight;

  return 0;
}

void FSvgWorker::Exit() {
    
}

void FSvgWorker::Stop() {
    
}