#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#define RENDER_SCALE 2.f
#define MODULE_DEPTH 2.f
#define OUTLINE_COLOR FLinearColor(1.f, 1.f, 0.f)

#define LIGHT_OBJECT ECC_GameTraceChannel1
#define PARAM_OBJECT ECC_GameTraceChannel2
#define PARAM_TRACE ECC_GameTraceChannel3
#define TELEPORT_TRACE ECC_GameTraceChannel4
#define INTERACTOR_OBJECT ECC_GameTraceChannel5

#define TAG_GRABBABLE FName("grabbable")
#define TAG_GRABBER FName("grabber")

#define TAG_INTERACTABLE FName("interactable")
#define TAG_INTERACTABLE_PARAM FName("interactable_param")
#define TAG_INTERACTABLE_PORT FName("interactable_port")

UENUM()
enum class PortType : int32
{
    Input,
    Output
}; 