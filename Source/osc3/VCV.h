#pragma once

#include "osc3.h"
#include "Math/Color.h"
#include <unordered_map>
#include <vector>

struct Vec2 {
  float x, y;
  
  Vec2& operator*=(const int& operand) {
    x *= operand;
    y *= operand;
    return *this;
  }

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

  Rect operator*=(const int& operand) {
    size *= operand;
    pos *= operand;
    return *this;
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
  bool visible{true};
  FLinearColor color;
  FLinearColor bgColor;
  bool transparent{true};

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
  FString displayValue;
  Rect box;
  
  float minValue;
  float maxValue;
  float defaultValue;
  float value;

  bool snap;
  bool visible{true};
  
  // Knob
  float minAngle;
  float maxAngle;
  
  // Slider
  Rect handleBox;
  Vec2 minHandlePos;
  Vec2 maxHandlePos;
  bool horizontal;
  float speed;
  
  // Switch/Button
  bool momentary;

  TMap<int32, VCVLight> Lights;
  
  TArray<FString> svgPaths;
  FLinearColor bodyColor;
  
  VCVParam() {}
  VCVParam(int32 _id) : id(_id) {}
  
  void merge(const VCVParam& other) {
    value = other.value;
    displayValue = other.displayValue;
    visible = other.visible;
  }
};

// enum PortType {
//   Input, Output
// };
struct PortIdentity {
  PortType type;
  int64_t moduleId = -1;
  int32 portId = -1;
  
  void nullify() {
    moduleId = -1;
    portId = -1;
  }

  bool isNull() {
    return moduleId == -1 && portId == -1;
  }

  PortIdentity() {}
  PortIdentity(PortType _type) : type(_type) {}
  PortIdentity(PortType _type, int64_t _moduleId, int32 _portId) : type(_type), moduleId(_moduleId), portId(_portId) {}
};
struct VCVPort {
  int32 id;
  PortType type;
  int64_t moduleId;
  FString name;
  FString description;
  Rect box;
  FString svgPath;
  FLinearColor bodyColor;

  PortIdentity getIdentity() {
    return PortIdentity(type, moduleId, id);
  }
  
  VCVPort() {}
  VCVPort(int32 _id, PortType _type) : id(_id), type(_type) {}
  VCVPort(int32 _id, PortType _type, int64_t _moduleId) : id(_id), type(_type), moduleId(_moduleId) {}
};

struct VCVCable {
  int64_t id = -1;
  TMap<PortType, PortIdentity> portIdentities = {
    { PortType::Input, PortIdentity(PortType::Input) },
    { PortType::Output, PortIdentity(PortType::Output) }
  };
  
  void setIdentity(PortIdentity identity) {
    portIdentities[identity.type] = identity;
  }

  PortIdentity getIdentity(PortType type) {
    return portIdentities[type];
  }

  void nullifyIdentity(PortType type) {
    portIdentities[type].nullify();
  }
  
  bool operator==(const VCVCable& other) {
    return id == other.id;
  }
  
  VCVCable() {}
  VCVCable(int64_t _id) : id(_id) {}
};

struct VCVDisplay {
  Rect box;
  
  VCVDisplay() {}
  VCVDisplay(Rect displayBox) : box(displayBox) {}
};

struct VCVModule {
  int64_t id;
  FString brand;
  FString name;
  FString description;
  Rect box;
  FString panelSvgPath;
  FLinearColor bodyColor;

  FString slug;
  FString pluginSlug;

  TMap<int32, VCVParam> Params;
  TMap<int32, VCVPort> Inputs;
  TMap<int32, VCVPort> Outputs;
  TMap<int32, VCVLight> Lights;
  std::vector<VCVDisplay> Displays;

	VCVModule() {}
  VCVModule(int64_t moduleId, FString moduleBrand, FString moduleName, FString modelDescription, Rect panelBox, FString svgPath)
    : id(moduleId), brand(moduleBrand), name(moduleName), description(modelDescription), box(panelBox), panelSvgPath(svgPath) {}
};

enum VCVMenuItemType {
  LABEL,
  ACTION,
  SUBMENU,
  RANGE,
  DIVIDER,
  // the following are specific to the unreal side and should stay last
  BACK
};

struct VCVMenuItem {
  int64_t moduleId;
  int menuId, index;
  VCVMenuItemType type;

  FString text;
  bool checked{false};
  bool disabled{false};

  float quantityValue{0.f},
    quantityMinValue{0.f}, quantityMaxValue{1.f}, quantityDefaultValue{0.f};
  FString quantityLabel{""},
    quantityUnit{""};
  
  VCVMenuItem() {}
  VCVMenuItem(int64_t _moduleId, int _menuId, int _index)
    : moduleId(_moduleId), menuId(_menuId), index(_index) {}
};

struct VCVMenu {
  int64_t moduleId;
  int id, parentMenuId{-1}, parentItemIndex{-1};
  TMap<int, VCVMenuItem> MenuItems;
  
  VCVMenu(int64_t _moduleId, int _id) : moduleId(_moduleId), id(_id) {}
};

typedef TMap<int, VCVMenu> ModuleMenuMap;