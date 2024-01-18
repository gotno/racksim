#pragma once

#include "osc3.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Grabbable.generated.h"

UINTERFACE(MinimalAPI)
class UGrabbable : public UInterface {
	GENERATED_BODY()
};

class OSC3_API IGrabbable {
	GENERATED_BODY()

public:
  virtual void EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  virtual void AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation);
  virtual void ReleaseGrab();
  virtual void SetHighlighted(bool bHighlighted, FLinearColor OutlineColor = OUTLINE_COLOR);

  bool bGrabEngaged{false};

  FVector GrabOffset;
  FVector LastGrabbedLocation;
  FRotator LastGrabbedRotation;
  FVector LastLocationDelta;
};