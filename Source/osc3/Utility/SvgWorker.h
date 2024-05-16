#pragma once

#include "CoreMinimal.h"

class UTexture2D;

class FSvgWorker : public FRunnable {

public:
  FSvgWorker();
  virtual ~FSvgWorker();

  void MakeTexture(FString& inFilepath, UTexture2D* inTextureTarget, float inScale);

  UPROPERTY()
  UTexture2D* TextureTarget{nullptr};
  FString Filepath;
  float scale{1.f};
  bool bIsFinished{false};

  virtual bool Init() override;
  virtual uint32 Run() override;

private:
  FRunnableThread* Thread{nullptr};
  void Cleanup();
};