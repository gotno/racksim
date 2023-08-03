#include "OSCController.h"
#include "osc3GameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "OSCManager.h"

AOSCController::AOSCController() {
  OSCClient = UOSCManager::CreateOSCClient("127.0.0.1", 7000, TEXT("OSCCtrlClient"), this);
  OSCServer = UOSCManager::CreateOSCServer("127.0.0.1", 7001, false, false, TEXT("OSCCtrlServer"), this);
}

void AOSCController::Init() {
  if (Aosc3GameModeBase* gm = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this))) {
    gameMode = gm;
  } else {
    UE_LOG(LogTemp, Error, TEXT("no gamemode for OSCctrl"));
    return;
  }

  AddRoute("/modules/add/*", FName(TEXT("AddModule")));
  AddRoute("/modules/param/add/*", FName(TEXT("AddParam")));
  AddRoute("/modules/input/add/*", FName(TEXT("AddInput")));
  AddRoute("/modules/output/add/*", FName(TEXT("AddOutput")));
  AddRoute("/modules/light/add/*", FName(TEXT("AddLight")));
  AddRoute("/modules/display/add/*", FName(TEXT("AddDisplay")));
  AddRoute("/cables/add/*", FName(TEXT("AddCable")));

  AddRoute("/modules/light/update/*", FName(TEXT("UpdateLight")));

  AddRoute("/module_sync_complete/*", FName(TEXT("ModuleSyncComplete")));

  // OSCServer->OnOscBundleReceived.AddDynamic(this, &AOSCController::TestBundle);
  // OSCServer->OnOscMessageReceived.AddDynamic(this, &AOSCController::TestMessage);

  OSCServer->Listen();
}

void AOSCController::AddRoute(const FString &AddressPattern, const FName &MethodName) {
  FOSCDispatchMessageEventBP Event;
  Event.BindUFunction(this, MethodName);
  OSCServer->BindEventToOnOSCAddressPatternMatchesPath(
    UOSCManager::ConvertStringToOSCAddress(AddressPattern),
    Event
  );
}

// void AOSCController::TestBundle(const FOSCBundle& InBundle, const FString& InIPAddress, int32 InPort) {
//   // IOSCPacket* packet = InBundle.GetPacket().Get();
// 	TSharedPtr<FOSCBundlePacket> BundlePacket = StaticCastSharedPtr<FOSCBundlePacket>(InBundle.GetPacket());
// 	FOSCBundlePacket::FPacketBundle Packets = BundlePacket->GetPackets();
//   UE_LOG(LogTemp, Warning, TEXT("Bundle received, num packets: %lld"), Packets.Num());
// 	for (TSharedPtr<IOSCPacket>& Packet : Packets) {
// 		if (Packet->IsMessage()) {
// 			// DispatchMessage(InIPAddress, InPort, FOSCMessage(Packet));
//       UE_LOG(LogTemp, Warning, TEXT("Packet is message: %s"), *FOSCMessage(Packet).GetAddress().GetFullPath());
// 		}
// 		else if (Packet->IsBundle()) {
//       UE_LOG(LogTemp, Warning, TEXT("Packet is bundle."));
//       TestBundle(InBundle, InIPAddress, InPort);
// 		}
// }

// void AOSCController::TestMessage(const FOSCMessage& InMessage, const FString& InIPAddress, int32 InPort) {
//   UE_LOG(LogTemp, Warning, TEXT("%s"), *InMessage.GetAddress().GetFullPath());
// }

void AOSCController::ModuleSyncComplete(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64 moduleId;
  if (ModuleGuard(message, moduleId)) return;

  UE_LOG(LogTemp, Warning, TEXT("Module Sync Complete %lld"), moduleId);

  gameMode->SpawnModule(Modules[moduleId]);
}

void AOSCController::AddModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64 moduleId;
  UOSCManager::GetInt64(message, 0, moduleId);
  if (Modules.Contains(moduleId)) {
    UE_LOG(LogTemp, Error, TEXT("received duplicate model %lld"), moduleId);
    return;
  }
  
  FString name;
  UOSCManager::GetString(message, 1, name);

  UE_LOG(LogTemp, Warning, TEXT("AddModule %lld %s"), moduleId, *name);

  FString description;
  UOSCManager::GetString(message, 2, description);
  
  Rect box;
  UOSCManager::GetFloat(message, 3, box.pos.x);
  UOSCManager::GetFloat(message, 4, box.pos.y);
  UOSCManager::GetFloat(message, 5, box.size.x);
  UOSCManager::GetFloat(message, 6, box.size.y);
  
  FString panelSvgPath;
  UOSCManager::GetString(message, 7, panelSvgPath);
  
  Modules.Add(moduleId, VCVModule(
      moduleId,
      name,
      description,
      box,
      panelSvgPath
  ));

  gameMode->RegisterSVG(panelSvgPath, box.size);
  
  NotifyReceived("module", moduleId);
}

void AOSCController::NotifyReceived(FString type, int64 outerId, int innerId) {
  // UE_LOG(LogTemp, Warning, TEXT("Sending NotifyReceived %s %lld:%d"), *type, outerId, innerId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/rx/")) + type);
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, outerId);
  UOSCManager::AddInt32(message, innerId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::NotifyResync() {
  UE_LOG(LogTemp, Warning, TEXT("Sending NotifyResync"));
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/sync")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendParamUpdate(int64 moduleId, int paramId, float value) {
  // UE_LOG(LogTemp, Warning, TEXT("send param update %lld:%d %f (%f/%f)"), moduleId, paramId, Modules[moduleId].Params[paramId].value, Modules[moduleId].Params[paramId].minValue, Modules[moduleId].Params[paramId].maxValue);

  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/update/param")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, moduleId);
  UOSCManager::AddInt32(message, paramId);
  UOSCManager::AddFloat(message, value);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::CreateCable(int64 inputModuleId, int64 outputModuleId, int inputPortId, int outputPortId) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/create/cable")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, inputModuleId);
  UOSCManager::AddInt64(message, outputModuleId);
  UOSCManager::AddInt32(message, inputPortId);
  UOSCManager::AddInt32(message, outputPortId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::DestroyCable(int64 cableId) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/destroy/cable")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, cableId);
  
  OSCClient->SendOSCMessage(message);
}

bool AOSCController::ModuleGuard(const FOSCMessage &message, int64_t &moduleId) {
  UOSCManager::GetInt64(message, 0, moduleId);

  if (!Modules.Contains(moduleId)) {
    UE_LOG(LogTemp, Warning, TEXT("can't find module %lld"), moduleId);
    return true;
  }
  
  return false;
}

void AOSCController::AddParam(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddParam"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 paramId;
  UOSCManager::GetInt32(message, 1, paramId);
  Modules[moduleId].Params.Add(paramId, VCVParam(paramId));
  VCVParam& param = Modules[moduleId].Params[paramId];

  int32 type;
  UOSCManager::GetInt32(message, 2, type);
  param.type = static_cast<ParamType>(type);

  UOSCManager::GetString(message, 3, param.name);
  UOSCManager::GetString(message, 4, param.unit);
  UOSCManager::GetString(message, 5, param.description);

  UOSCManager::GetFloat(message, 6, param.box.pos.x);
  UOSCManager::GetFloat(message, 7, param.box.pos.y);
  UOSCManager::GetFloat(message, 8, param.box.size.x);
  UOSCManager::GetFloat(message, 9, param.box.size.y);

  UOSCManager::GetFloat(message, 10, param.minValue);
  UOSCManager::GetFloat(message, 11, param.maxValue);
  UOSCManager::GetFloat(message, 12, param.defaultValue);
  UOSCManager::GetFloat(message, 13, param.value);
  
  UOSCManager::GetBool(message, 14, param.snap);

  UOSCManager::GetFloat(message, 15, param.minAngle);
  UOSCManager::GetFloat(message, 16, param.maxAngle);
  param.minAngle = FMath::RadiansToDegrees(param.minAngle);
  param.maxAngle = FMath::RadiansToDegrees(param.maxAngle);

  UOSCManager::GetFloat(message, 17, param.minHandlePos.x);
  UOSCManager::GetFloat(message, 18, param.minHandlePos.y);
  UOSCManager::GetFloat(message, 19, param.maxHandlePos.x);
  UOSCManager::GetFloat(message, 20, param.maxHandlePos.y);
  UOSCManager::GetFloat(message, 21, param.handleBox.pos.x);
  UOSCManager::GetFloat(message, 22, param.handleBox.pos.y);
  UOSCManager::GetFloat(message, 23, param.handleBox.size.x);
  UOSCManager::GetFloat(message, 24, param.handleBox.size.y);

  UOSCManager::GetBool(message, 25, param.horizontal);

  UOSCManager::GetFloat(message, 26, param.speed);

  UOSCManager::GetBool(message, 27, param.latch);
  UOSCManager::GetBool(message, 28, param.momentary);
  UOSCManager::GetBool(message, 29, param.visible);

  FString svgPath;

  UOSCManager::GetString(message, 30, svgPath);
  param.svgPaths.Add(svgPath);
  gameMode->RegisterSVG(svgPath, param.box.size);

  UOSCManager::GetString(message, 31, svgPath);
  param.svgPaths.Add(svgPath);
  gameMode->RegisterSVG(svgPath, param.box.size);

  UOSCManager::GetString(message, 32, svgPath);
  param.svgPaths.Add(svgPath);
  gameMode->RegisterSVG(svgPath, param.box.size);

  UOSCManager::GetString(message, 33, svgPath);
  param.svgPaths.Add(svgPath);
  gameMode->RegisterSVG(svgPath, param.box.size);

  UOSCManager::GetString(message, 34, svgPath);
  param.svgPaths.Add(svgPath);
  gameMode->RegisterSVG(svgPath, param.box.size);

  NotifyReceived("param", moduleId, paramId);
}

void AOSCController::AddInput(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddInput"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 inputId;
  UOSCManager::GetInt32(message, 1, inputId);
  Modules[moduleId].Inputs.Add(inputId, VCVPort(inputId, PortType::Input, moduleId));
  VCVPort& input = Modules[moduleId].Inputs[inputId];

  UOSCManager::GetString(message, 2, input.name);
  UOSCManager::GetString(message, 3, input.description);

  UOSCManager::GetFloat(message, 4, input.box.pos.x);
  UOSCManager::GetFloat(message, 5, input.box.pos.y);
  UOSCManager::GetFloat(message, 6, input.box.size.x);
  UOSCManager::GetFloat(message, 7, input.box.size.y);

  UOSCManager::GetString(message, 8, input.svgPath);

  gameMode->RegisterSVG(input.svgPath, input.box.size);

  NotifyReceived("input", moduleId, inputId);
}

void AOSCController::AddOutput(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddOutput"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 outputId;
  UOSCManager::GetInt32(message, 1, outputId);
  Modules[moduleId].Outputs.Add(outputId, VCVPort(outputId, PortType::Output, moduleId));
  VCVPort& output = Modules[moduleId].Outputs[outputId];

  UOSCManager::GetString(message, 2, output.name);
  UOSCManager::GetString(message, 3, output.description);

  UOSCManager::GetFloat(message, 4, output.box.pos.x);
  UOSCManager::GetFloat(message, 5, output.box.pos.y);
  UOSCManager::GetFloat(message, 6, output.box.size.x);
  UOSCManager::GetFloat(message, 7, output.box.size.y);

  UOSCManager::GetString(message, 8, output.svgPath);

  gameMode->RegisterSVG(output.svgPath, output.box.size);

  NotifyReceived("output", moduleId, outputId);
}

void AOSCController::AddDisplay(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddDisplay"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  Rect displayBox;
  UOSCManager::GetFloat(message, 1, displayBox.pos.x);
  UOSCManager::GetFloat(message, 2, displayBox.pos.y);
  UOSCManager::GetFloat(message, 3, displayBox.size.x);
  UOSCManager::GetFloat(message, 4, displayBox.size.y);
  
  Modules[moduleId].Displays.emplace_back(displayBox);

  NotifyReceived("display", moduleId);
}

void AOSCController::AddLight(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddLight"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 paramId;
  UOSCManager::GetInt32(message, 1, paramId);

  int32 lightId;
  UOSCManager::GetInt32(message, 2, lightId);
  
  if (paramId == -1) {
    Modules[moduleId].Lights.Add(lightId, VCVLight(lightId, moduleId));
  } else {
    Modules[moduleId].Params[paramId].Lights.Add(lightId, VCVLight(lightId, moduleId));
  }

  VCVLight& light =
    paramId == -1
      ? Modules[moduleId].Lights[lightId]
      : Modules[moduleId].Params[paramId].Lights[lightId];

  if (paramId != -1) {
    ParamType& type = Modules[moduleId].Params[paramId].type;
    if (type == ParamType::Button || type == ParamType::Slider) light.transparent = true;
  }

  UOSCManager::GetFloat(message, 3, light.box.pos.x);
  UOSCManager::GetFloat(message, 4, light.box.pos.y);
  UOSCManager::GetFloat(message, 5, light.box.size.x);
  UOSCManager::GetFloat(message, 6, light.box.size.y);

  {
    float rgba_r, rgba_g, rgba_b, rgba_a;
    UOSCManager::GetFloat(message, 7, rgba_r);
    UOSCManager::GetFloat(message, 8, rgba_g);
    UOSCManager::GetFloat(message, 9, rgba_b);
    UOSCManager::GetFloat(message, 10, rgba_a);
    light.color = FLinearColor(rgba_r, rgba_g, rgba_b, rgba_a);
  }
  {
    float rgba_r, rgba_g, rgba_b, rgba_a;
    UOSCManager::GetFloat(message, 11, rgba_r);
    UOSCManager::GetFloat(message, 12, rgba_g);
    UOSCManager::GetFloat(message, 13, rgba_b);
    UOSCManager::GetFloat(message, 14, rgba_a);
    light.bgColor = FLinearColor(rgba_r, rgba_g, rgba_b, rgba_a);
  }

  int32 shape;
  UOSCManager::GetInt32(message, 15, shape);
  light.shape = static_cast<LightShape>(shape);

  NotifyReceived("module_light", moduleId, lightId);
}

void AOSCController::AddCable(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddCable"));

  int64_t cableId;
  UOSCManager::GetInt64(message, 0, cableId);
  
  Cables.Add(cableId, VCVCable(cableId));

  VCVCable& cable = Cables[cableId];

  UOSCManager::GetInt64(message, 1, cable.portIdentities[PortType::Input].moduleId);
  UOSCManager::GetInt64(message, 2, cable.portIdentities[PortType::Output].moduleId);
  UOSCManager::GetInt32(message, 3, cable.portIdentities[PortType::Input].portId);
  UOSCManager::GetInt32(message, 4, cable.portIdentities[PortType::Output].portId);

  NotifyReceived("cable", cableId);

  gameMode->QueueCableSpawn(Cables[cableId]);
}

void AOSCController::UpdateLight(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("UpdateLight"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 lightId;
  UOSCManager::GetInt32(message, 1, lightId);
  
  float r, g, b, a;
  UOSCManager::GetFloat(message, 2, r);
  UOSCManager::GetFloat(message, 3, g);
  UOSCManager::GetFloat(message, 4, b);
  UOSCManager::GetFloat(message, 5, a);

  gameMode->UpdateLight(moduleId, lightId, FLinearColor(r, g, b, a));
}

// void AOSCController::PrintVCVModule(VCVModule vcv_module) {
//   UE_LOG(LogTemp, Warning, TEXT("%lld %s"), vcv_module.id, *vcv_module.name);
//   UE_LOG(LogTemp, Warning, TEXT("  pos: %fx/%fy, size: %fx/%fy"), vcv_module.box.pos.x, vcv_module.box.pos.y, vcv_module.box.size.x, vcv_module.box.size.y);
// }

void AOSCController::LogOSC(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  UE_LOG(
    LogTemp,
    Error,
    TEXT("MATCHING PATTERN %s TO THING %s"),
    *AddressPattern.GetFullPath(),
    *message.GetAddress().GetFullPath()
  );
}