#pragma once

#include "Math/Color.h"
#include <unordered_map>
#include <vector>

struct Vec2 {
  float x, y;

  Vec2() : x(0.f), y(0.f) {};
  Vec2(float _x, float _y) : x(_x), y(_y) {};
};

struct Rect {
  Vec2 pos;
  Vec2 size;

  FVector extent() {
    return FVector(0, size.x / 2, size.y / 2);
  }

  FVector location() {
    return FVector(0, pos.x, pos.y);
  }
  
  Rect() {}
  Rect(Vec2 _pos, Vec2 _size) : pos(_pos), size(_size) {}
};

enum LightShape {
  Round,
  Rectangle
};
struct VCVLight {
  int32 id;
  int64_t moduleId;
  int paramId = -1;
  Rect box;
  LightShape shape;
  FLinearColor color;
  FLinearColor bgColor;
  bool transparent = false;

  VCVLight() {}
  VCVLight(int32 _id, int64_t _moduleId) : id(_id), moduleId(_moduleId) {}
};

enum ParamType {
  Knob, Slider, Button, Switch
};
struct VCVParam {
  int32 id;
  ParamType type;
  FString name;
  FString unit;
  FString description;
  Rect box;
  
  float minValue;
  float maxValue;
  float defaultValue;
  float value;

  bool snap;
  
  // Knob
  float minAngle;
  float maxAngle;
  
  // Slider
  Rect handleBox;
  Vec2 minHandlePos;
  Vec2 maxHandlePos;
  bool horizontal;
  float speed;
  
  // Switch
  bool latch;
  bool momentary;

  TMap<int32, VCVLight> Lights;
  int32 frameCount;
  
  // internal operations, not useful?
  // float displayBase;
  // float displayMultiplier;
  // float displayOffset;
  VCVParam() {}
  VCVParam(int32 _id) : id(_id) {}
};

enum PortType {
  Input, Output
};
struct PortIdentity {
  PortType type;
  int64_t moduleId;
  int32 portId;

  PortIdentity() {}
  PortIdentity(PortType _type) : type(_type) {}
};
struct VCVPort {
  int32 id;
  PortType type;
  // PortIdentity identity;
  FString name;
  FString description;
  Rect box;
  
  VCVPort() {}
  VCVPort(int32 _id, PortType _type) : id(_id), type(_type) {}
};

struct VCVDisplay {
  Rect box;
  
  VCVDisplay() {}
  VCVDisplay(Rect displayBox) : box(displayBox) {}
};

struct VCVModule {
  int64_t id;
  FString name;
  FString description;
  Rect box;

  TMap<int32, VCVParam> Params;
  TMap<int32, VCVPort> Inputs;
  TMap<int32, VCVPort> Outputs;
  TMap<int32, VCVLight> Lights;
  std::vector<VCVDisplay> Displays;

	VCVModule() {}
  VCVModule(int64_t moduleId, FString moduleName, FString modelDescription, Rect panelBox)
    : id(moduleId), name(moduleName), description(modelDescription), box(panelBox) {}
};

struct VCVCable {
  int64_t id = -1;
  TMap<PortType, PortIdentity> portIdentities = {
    { PortType::Input, PortIdentity(PortType::Input) },
    { PortType::Output, PortIdentity(PortType::Output) }
  };
  
  PortIdentity getInputIdentity() {
    return portIdentities[PortType::Input];
  }

  PortIdentity getOutputIdentity() {
    return portIdentities[PortType::Output];
  }
  
  bool operator==(const VCVCable& other) {
    return id == other.id;
  }
  // int64_t inputModuleId, outputModuleId;
  // int32 inputPortId, outputPortId;
  
  VCVCable() {}
  VCVCable(int64_t _id) : id(_id) {}
};