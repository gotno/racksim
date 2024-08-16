#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#define DEFAULT_RENDER_SCALE 1.667f
#define UNSCALED_MODULE_HEIGHT 12.869334f
#define UNSCALED_MODULE_DEPTH 2.f

#define OUTLINE_COLOR FLinearColor(0.724268f, 0.385187f, 0.503043f)

// percent
#define DEFAULT_CABLE_OPACITY 60.f
#define DEFAULT_CABLE_TENSION 40.f

// degrees
#define MIN_SUN_ANGLE 170.f
#define MAX_SUN_ANGLE 370.f
#define DEFAULT_SUN_ANGLE 190.f

// candelas
#define DEFAULT_ROOM_BRIGHTNESS 4.f
#define DEFAULT_ROOM_BRIGHTNESS_WITH_SUN 0.5f
#define MAX_ROOM_BRIGHTNESS 8.f

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

#define KNOB_ALTER_RATIO 2.f
#define KNOB_ALTER_RATIO_PRECISE 0.8f

#define LIGHT_OBJECT ECC_GameTraceChannel1
#define PARAM_OBJECT ECC_GameTraceChannel2
#define PORT_OBJECT ECC_GameTraceChannel7
#define CABLE_END_OBJECT ECC_GameTraceChannel8
#define DISPLAY_OBJECT ECC_GameTraceChannel6
#define LEFT_SNAP_COLLIDER_OBJECT ECC_GameTraceChannel5
#define RIGHT_SNAP_COLLIDER_OBJECT ECC_GameTraceChannel9

#define PARAM_TRACE ECC_GameTraceChannel3
#define TELEPORT_TRACE ECC_GameTraceChannel4
#define WIDGET_TRACE ECC_GameTraceChannel10

#define TAG_INTERACTABLE FName("interactable")
#define TAG_INTERACTABLE_PARAM FName("interactable_param")
#define TAG_INTERACTABLE_PORT FName("interactable_port")

#define LOADING_PATCH_LABEL FString("initializing simulation")

UENUM()
enum class PortType : int32 {
  Input,
  Output,
  Any
};

UENUM()
enum class FSnapModeSide : int32 {
  Left,
  Right,
  Both,
  None
};

UENUM()
enum class EFileType : int32 {
  File,
  Directory
};

UENUM()
enum class EFileIcon : int32 {
  File,
  Directory,
  None
};

#define ODB(Format, ...) if(GEngine){ GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, FString::Printf(TEXT(Format), ##__VA_ARGS__), false); }