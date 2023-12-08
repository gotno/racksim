#include "OSCController.h"

#include "osc3.h"
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

  AddRoute("/param/sync/*", FName(TEXT("SyncParam")));
  
  AddRoute("/library/plugin/add/*", FName(TEXT("AddLibraryPlugin")));
  AddRoute("/library/module/add/*", FName(TEXT("AddLibraryModule")));
  AddRoute("/library/module_tag/add/*", FName(TEXT("AddLibraryModuleTag")));
  AddRoute("/library/tag/add/*", FName(TEXT("AddLibraryTag")));

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

  // UE_LOG(LogTemp, Warning, TEXT("Module Sync Complete %lld"), moduleId);

  gameMode->SpawnModule(Modules[moduleId]);
}

void AOSCController::AddModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64 moduleId;
  UOSCManager::GetInt64(message, 0, moduleId);
  if (Modules.Contains(moduleId)) {
    UE_LOG(LogTemp, Error, TEXT("received duplicate model %lld"), moduleId);
    return;
  }
  
  FString brand;
  UOSCManager::GetString(message, 1, brand);
  
  FString name;
  UOSCManager::GetString(message, 2, name);

  UE_LOG(LogTemp, Warning, TEXT("AddModule %lld %s:%s"), moduleId, *brand, *name);

  FString description;
  UOSCManager::GetString(message, 3, description);

  FString slug;
  UOSCManager::GetString(message, 4, slug);
  FString pluginSlug;
  UOSCManager::GetString(message, 5, pluginSlug);
  
  Rect box;
  UOSCManager::GetFloat(message, 6, box.pos.x);
  UOSCManager::GetFloat(message, 7, box.pos.y);
  UOSCManager::GetFloat(message, 8, box.size.x);
  UOSCManager::GetFloat(message, 9, box.size.y);
  box *= RENDER_SCALE;
  
  FString panelSvgPath;
  UOSCManager::GetString(message, 10, panelSvgPath);
  gameMode->RegisterSVG(panelSvgPath, box.size);
  
  Modules.Add(moduleId, VCVModule(
      moduleId,
      brand,
      name,
      description,
      box,
      panelSvgPath
  ));

  UOSCManager::GetFloat(message, 11, Modules[moduleId].bodyColor.R);
  UOSCManager::GetFloat(message, 12, Modules[moduleId].bodyColor.G);
  UOSCManager::GetFloat(message, 13, Modules[moduleId].bodyColor.B);
  
  Modules[moduleId].slug = slug;
  Modules[moduleId].pluginSlug = pluginSlug;

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

void AOSCController::CreateModule(FString pluginSlug, FString moduleSlug) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /create/module %s:%s"), *pluginSlug, *moduleSlug);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/create/module")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddString(message, pluginSlug);
  UOSCManager::AddString(message, moduleSlug);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SetModuleFavorite(FString pluginSlug, FString moduleSlug, bool bFavorite) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /favorite %s:%s-%d"), *pluginSlug, *moduleSlug, bFavorite);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/favorite")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddString(message, pluginSlug);
  UOSCManager::AddString(message, moduleSlug);
  UOSCManager::AddBool(message, bFavorite);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::DestroyModule(int64 moduleId) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /destroy/module %lld"), moduleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/destroy/module")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, moduleId);

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
  UOSCManager::GetString(message, 4, param.displayValue);

  UOSCManager::GetFloat(message, 5, param.box.pos.x);
  UOSCManager::GetFloat(message, 6, param.box.pos.y);
  UOSCManager::GetFloat(message, 7, param.box.size.x);
  UOSCManager::GetFloat(message, 8, param.box.size.y);
  param.box *= RENDER_SCALE;

  UOSCManager::GetFloat(message, 9, param.minValue);
  UOSCManager::GetFloat(message, 10, param.maxValue);
  UOSCManager::GetFloat(message, 11, param.defaultValue);
  UOSCManager::GetFloat(message, 12, param.value);
  
  UOSCManager::GetBool(message, 13, param.snap);

  UOSCManager::GetFloat(message, 14, param.minAngle);
  UOSCManager::GetFloat(message, 15, param.maxAngle);
  param.minAngle = FMath::RadiansToDegrees(param.minAngle);
  param.maxAngle = FMath::RadiansToDegrees(param.maxAngle);

  UOSCManager::GetFloat(message, 16, param.minHandlePos.x);
  UOSCManager::GetFloat(message, 17, param.minHandlePos.y);
  param.minHandlePos *= RENDER_SCALE;
  UOSCManager::GetFloat(message, 18, param.maxHandlePos.x);
  UOSCManager::GetFloat(message, 19, param.maxHandlePos.y);
  param.maxHandlePos *= RENDER_SCALE;
  UOSCManager::GetFloat(message, 20, param.handleBox.pos.x);
  UOSCManager::GetFloat(message, 21, param.handleBox.pos.y);
  UOSCManager::GetFloat(message, 22, param.handleBox.size.x);
  UOSCManager::GetFloat(message, 23, param.handleBox.size.y);
  param.handleBox *= RENDER_SCALE;

  UOSCManager::GetBool(message, 24, param.horizontal);

  UOSCManager::GetFloat(message, 25, param.speed);

  UOSCManager::GetBool(message, 26, param.momentary);
  UOSCManager::GetBool(message, 27, param.visible);

  param.svgPaths.SetNum(5);

  UOSCManager::GetString(message, 28, param.svgPaths[0]);
  gameMode->RegisterSVG(param.svgPaths[0], param.box.size);

  UOSCManager::GetString(message, 29, param.svgPaths[1]);
  gameMode->RegisterSVG(param.svgPaths[1], param.box.size);

  UOSCManager::GetString(message, 30, param.svgPaths[2]);
  gameMode->RegisterSVG(param.svgPaths[2], param.box.size);

  UOSCManager::GetString(message, 31, param.svgPaths[3]);
  gameMode->RegisterSVG(param.svgPaths[3], param.box.size);

  UOSCManager::GetString(message, 32, param.svgPaths[4]);
  gameMode->RegisterSVG(param.svgPaths[4], param.box.size);

  UOSCManager::GetFloat(message, 33, param.bodyColor.R);
  UOSCManager::GetFloat(message, 34, param.bodyColor.G);
  UOSCManager::GetFloat(message, 35, param.bodyColor.B);
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
  input.box *= RENDER_SCALE;

  UOSCManager::GetString(message, 8, input.svgPath);

  gameMode->RegisterSVG(input.svgPath, input.box.size);

  UOSCManager::GetFloat(message, 9, input.bodyColor.R);
  UOSCManager::GetFloat(message, 10, input.bodyColor.G);
  UOSCManager::GetFloat(message, 11, input.bodyColor.B);
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
  output.box *= RENDER_SCALE;

  UOSCManager::GetString(message, 8, output.svgPath);

  gameMode->RegisterSVG(output.svgPath, output.box.size);

  UOSCManager::GetFloat(message, 9, output.bodyColor.R);
  UOSCManager::GetFloat(message, 10, output.bodyColor.G);
  UOSCManager::GetFloat(message, 11, output.bodyColor.B);
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
  displayBox *= RENDER_SCALE;
  
  Modules[moduleId].Displays.emplace_back(displayBox);
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
  light.box *= RENDER_SCALE;

  UOSCManager::GetFloat(message, 7, light.color.R);
  UOSCManager::GetFloat(message, 8, light.color.G);
  UOSCManager::GetFloat(message, 9, light.color.B);
  UOSCManager::GetFloat(message, 10, light.color.A);

  UOSCManager::GetFloat(message, 11, light.bgColor.R);
  UOSCManager::GetFloat(message, 12, light.bgColor.G);
  UOSCManager::GetFloat(message, 13, light.bgColor.B);
  UOSCManager::GetFloat(message, 14, light.bgColor.A);

  int32 shape;
  UOSCManager::GetInt32(message, 15, shape);
  light.shape = static_cast<LightShape>(shape);

  UOSCManager::GetBool(message, 16, light.visible);
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
  
  FLinearColor color;
  UOSCManager::GetFloat(message, 2, color.R);
  UOSCManager::GetFloat(message, 3, color.G);
  UOSCManager::GetFloat(message, 4, color.B);
  UOSCManager::GetFloat(message, 5, color.A);

  gameMode->UpdateLight(moduleId, lightId, color);
}

void AOSCController::SyncParam(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;
  
  int32 paramId;
  FString displayValue;
  UOSCManager::GetInt32(message, 1, paramId);
  UOSCManager::GetString(message, 2, displayValue);
  
  gameMode->UpdateParamDisplayValue(moduleId, paramId, displayValue);
}

void AOSCController::AddLibraryPlugin(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  FString name, slug;
  UOSCManager::GetString(message, 0, name);
  UOSCManager::GetString(message, 1, slug);
  
  Library.Plugins.Add(slug, VCVPluginInfo(name, slug));
  // UE_LOG(LogTemp, Warning, TEXT("added library plugin %s:%s"), *name, *slug);
}

void AOSCController::AddLibraryModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  FString pluginSlug, name, slug, description;
  bool favorite;
  UOSCManager::GetString(message, 0, pluginSlug);
  UOSCManager::GetString(message, 1, name);
  UOSCManager::GetString(message, 2, slug);
  UOSCManager::GetString(message, 3, description);
  UOSCManager::GetBool(message, 4, favorite);

  Library.Plugins[pluginSlug].Modules.Add(slug, VCVModuleInfo(name, slug, description, favorite));
  
  gameMode->UpdateLibrary(Library);
  // UE_LOG(LogTemp, Warning, TEXT("  added library module %s:%s (%s)"), *name, *slug, *description);
}

void AOSCController::AddLibraryModuleTag(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  FString pluginSlug, moduleSlug;
  int32 tagId;

  UOSCManager::GetString(message, 0, pluginSlug);
  UOSCManager::GetString(message, 1, moduleSlug);
  UOSCManager::GetInt32(message, 2, tagId);
  
  Library.Plugins[pluginSlug].Modules[moduleSlug].Tags.Add(tagId);
  Library.Plugins[pluginSlug].ModuleTags.Add(tagId);

  gameMode->UpdateLibrary(Library);
  // UE_LOG(LogTemp, Warning, TEXT("    added module tag %d"), tagId);
}

void AOSCController::AddLibraryTag(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int32 tagId;
  FString tagName;

  UOSCManager::GetInt32(message, 0, tagId);
  UOSCManager::GetString(message, 1, tagName);
  
  Library.TagNames.Add(tagId, tagName);

  gameMode->UpdateLibrary(Library);
  // UE_LOG(LogTemp, Warning, TEXT("added library tag %d:%s"), tagId, *tagName);
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