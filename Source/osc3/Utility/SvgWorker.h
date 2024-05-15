#pragma once

#include "CoreMinimal.h"

class FSvgWorker : public FRunnable {

public:
  FSvgWorker(FString inFilepath);
  virtual ~FSvgWorker();

  FString Filepath;
  int width{-1};
  int height{-1};
  unsigned char* rgba;

  virtual bool Init() override;
  virtual uint32 Run() override;
  virtual void Exit() override;
  virtual void Stop() override;
  
private:
  FRunnableThread* Thread;
};