#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#define DEFAULT_RENDER_SCALE 2.f
#define RENDER_SCALE 2.f

#define UNSCALED_MODULE_HEIGHT 12.869334f
#define MODULE_DEPTH 2.f

#define OUTLINE_COLOR FLinearColor(0.724268f, 0.385187f, 0.503043f)
#define CABLE_OPACITY 0.6f

#define GRABBABLE_ROTATION_SMOOTHING_FACTOR_DEFAULT 0.15f
#define GRABBABLE_ROTATION_SMOOTHING_FACTOR_MIN 0.05f
#define GRABBABLE_ROTATION_SMOOTHING_FACTOR_MAX 1.f
#define GRABBABLE_LOCATION_SMOOTHING_FACTOR_DEFAULT 0.3f
#define GRABBABLE_LOCATION_SMOOTHING_FACTOR_MIN 0.05f
#define GRABBABLE_LOCATION_SMOOTHING_FACTOR_MAX 1.f
#define WORLD_ROTATION_SMOOTHING_FACTOR_DEFAULT 0.3f
#define WORLD_ROTATION_SMOOTHING_FACTOR_MIN 0.05f
#define WORLD_ROTATION_SMOOTHING_FACTOR_MAX 1.f
#define WORLD_TRANSLATION_SMOOTHING_FACTOR_DEFAULT 0.3f
#define WORLD_TRANSLATION_SMOOTHING_FACTOR_MIN 0.05f
#define WORLD_TRANSLATION_SMOOTHING_FACTOR_MAX 1.f

#define LIGHT_OBJECT ECC_GameTraceChannel1
#define PARAM_OBJECT ECC_GameTraceChannel2
#define PORT_OBJECT ECC_GameTraceChannel7
#define CABLE_END_OBJECT ECC_GameTraceChannel8
#define INTERACTOR_OBJECT ECC_GameTraceChannel5
#define DISPLAY_OBJECT ECC_GameTraceChannel6

#define PARAM_TRACE ECC_GameTraceChannel3
#define TELEPORT_TRACE ECC_GameTraceChannel4

#define TAG_INTERACTABLE FName("interactable")
#define TAG_INTERACTABLE_PARAM FName("interactable_param")
#define TAG_INTERACTABLE_PORT FName("interactable_port")

UENUM()
enum class PortType : int32
{
    Input,
    Output,
    Any
};

#define ODB(Format, ...) if(GEngine){ GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT(Format), ##__VA_ARGS__), false); }