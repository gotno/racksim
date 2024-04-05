#include "OSCController.h"

#include "osc3.h"
#include "osc3GameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "OSCManager.h"

AOSCController::AOSCController() {
}

void AOSCController::Init() {
  OSCClient = UOSCManager::CreateOSCClient(TEXT("127.0.0.1"), RackClientPort, TEXT("OSCCtrlClient"), this);
  OSCServer = UOSCManager::CreateOSCServer(TEXT("127.0.0.1"), ServerPort, false, false, TEXT("OSCCtrlServer"), this);

  if (Aosc3GameModeBase* gm = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this))) {
    GameMode = gm;
  } else {
    UE_LOG(LogTemp, Error, TEXT("no gamemode for OSCctrl"));
    return;
  }

  AddRoute("/modules/add/*", FName(TEXT("AddModule")));
  AddRoute("/modules/destroy/*", FName(TEXT("DestroyModule")));
  AddRoute("/modules/param/add/*", FName(TEXT("AddParam")));
  AddRoute("/modules/input/add/*", FName(TEXT("AddInput")));
  AddRoute("/modules/output/add/*", FName(TEXT("AddOutput")));
  AddRoute("/modules/light/add/*", FName(TEXT("AddLight")));
  AddRoute("/modules/display/add/*", FName(TEXT("AddDisplay")));
  AddRoute("/cables/add/*", FName(TEXT("AddCable")));

  AddRoute("/modules/light/update/*", FName(TEXT("UpdateLight")));

  AddRoute("/module_sync_complete/*", FName(TEXT("ModuleSyncComplete")));

  AddRoute("/param/sync/*", FName(TEXT("SyncParam")));
  
  AddRoute("/library/json_path/*", FName(TEXT("SetLibraryJsonPath")));

  AddRoute("/menu/item/add/*", FName(TEXT("AddContextMenuItem")));
  AddRoute("/menu/synced/*", FName(TEXT("MenuSynced")));

  AddRoute("/set_rack_server_port/*", FName(TEXT("SetClientPort")));

  // OSCServer->OnOscBundleReceived.AddDynamic(this, &AOSCController::TestBundle);
  // OSCServer->OnOscMessageReceived.AddDynamic(this, &AOSCController::TestMessage);

  OSCServer->Listen();

  SyncPorts();

  bRunning = true;
}

void AOSCController::AddRoute(const FString &AddressPattern, const FName &MethodName) {
  FOSCDispatchMessageEventBP Event;
  Event.BindUFunction(this, MethodName);
  OSCServer->BindEventToOnOSCAddressPatternMatchesPath(
    UOSCManager::ConvertStringToOSCAddress(AddressPattern),
    Event
  );
}

void AOSCController::SyncPorts() {
  RackClientPort = MinRackClientPort;

  GetWorld()->GetTimerManager().SetTimer(
    hSyncPortTimer,
    this,
    &AOSCController::SendServerPort,
    0.05f, // 50 milliseconds
    true // loop
  );
}

void AOSCController::SendServerPort() {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString("/set_unreal_server_port"));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt32(message, ServerPort);

  // UE_LOG(LogTemp, Warning, TEXT("sending /set_unreal_server_port (%d) on port %d"), ServerPort, RackClientPort);
  OSCClient->SendOSCMessage(message);

  if (++RackClientPort > MaxRackClientPort) RackClientPort = MinRackClientPort;
  OSCClient->SetSendIPAddress(TEXT("127.0.0.1"), RackClientPort);
}

void AOSCController::SetClientPort(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int clientPort;
  UOSCManager::GetInt32(message, 0, clientPort);

  // UE_LOG(LogTemp, Warning, TEXT("received SetClientPort %d"), clientPort);
  GetWorld()->GetTimerManager().ClearTimer(hSyncPortTimer);

  RackClientPort = clientPort;
  OSCClient->SetSendIPAddress(TEXT("127.0.0.1"), RackClientPort);

  // wait a moment, request sync
  FTimerHandle resyncHandle;
  GetWorld()->GetTimerManager().SetTimer(
    resyncHandle,
    this,
    &AOSCController::NotifyResync,
    1.f, // 1 second
    false // loop
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

  GameMode->SpawnModule(Modules[moduleId]);
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
  GameMode->RegisterSVG(panelSvgPath, box.size);
  
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

  UOSCManager::GetInt32(message, 14, Modules[moduleId].returnId);
  UOSCManager::GetInt64(message, 15, Modules[moduleId].leftExpanderId);
  UOSCManager::GetInt64(message, 16, Modules[moduleId].rightExpanderId);
}

void AOSCController::DestroyModule(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64 moduleId;
  UOSCManager::GetInt64(message, 0, moduleId);

  UE_LOG(LogTemp, Warning, TEXT("DestroyModule %lld"), moduleId);

  GameMode->DestroyModule(moduleId, false);
}

void AOSCController::NotifyReceived(FString Type, int64 OuterId, int InnerId) {
  // UE_LOG(LogTemp, Warning, TEXT("Sending NotifyReceived %s %lld:%d"), *Type, OuterId, InnerId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/rx/")) + Type);
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, OuterId);
  UOSCManager::AddInt32(message, InnerId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::NotifyResync() {
  UE_LOG(LogTemp, Warning, TEXT("Sending NotifyResync"));
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/sync")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendParamUpdate(int64 ModuleId, int ParamId, float Value) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/update/param")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, ModuleId);
  UOSCManager::AddInt32(message, ParamId);
  UOSCManager::AddFloat(message, Value);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendCreateModule(const FString& PluginSlug, const FString& ModuleSlug, const int& ReturnId) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /create/module %s:%s"), *PluginSlug, *ModuleSlug);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/create/module")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddString(message, PluginSlug);
  UOSCManager::AddString(message, ModuleSlug);
  UOSCManager::AddInt32(message, ReturnId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /favorite %s:%s-%d"), *PluginSlug, *ModuleSlug, bFavorite);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/favorite")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddString(message, PluginSlug);
  UOSCManager::AddString(message, ModuleSlug);
  UOSCManager::AddBool(message, bFavorite);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendDestroyModule(int64 ModuleId) {
  UE_LOG(LogTemp, Warning, TEXT("Sending /destroy/module %lld"), ModuleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/destroy/module")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, ModuleId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendModuleDiffRequest(const int64_t& ModuleId) const {
  UE_LOG(LogTemp, Warning, TEXT("Sending /diff/module %lld"), ModuleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/diff/module")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, ModuleId);

  OSCClient->SendOSCMessage(message);
};

void AOSCController::CreateCable(int64 InputModuleId, int64 OutputModuleId, int InputPortId, int OutputPortId) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/create/cable")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, InputModuleId);
  UOSCManager::AddInt64(message, OutputModuleId);
  UOSCManager::AddInt32(message, InputPortId);
  UOSCManager::AddInt32(message, OutputPortId);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendAutosaveAndExit() {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/autosave_and_exit")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  OSCClient->SendOSCMessage(message);
}

void AOSCController::SendArrangeModules(int64 LeftModuleId, int64 RightModuleId, bool bAttach) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/arrange_modules")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  UOSCManager::AddInt64(message, LeftModuleId);
  UOSCManager::AddInt64(message, RightModuleId);
  UOSCManager::AddBool(message, bAttach);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::DestroyCable(int64 CableId) {
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/destroy/cable")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);
  UOSCManager::AddInt64(message, CableId);
  
  OSCClient->SendOSCMessage(message);
}

void AOSCController::RequestMenu(const FVCVMenu& Menu) const {
  UE_LOG(LogTemp, Warning, TEXT("Sending /get_menu %lld"), Menu.moduleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/get_menu")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  UOSCManager::AddInt64(message, Menu.moduleId);
  UOSCManager::AddInt32(message, Menu.id);
  UOSCManager::AddInt32(message, Menu.parentMenuId);
  UOSCManager::AddInt32(message, Menu.parentItemIndex);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::ClickMenuItem(const FVCVMenuItem& MenuItem) const {
  UE_LOG(LogTemp, Warning, TEXT("Sending /click_menu_item %lld"), MenuItem.moduleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/click_menu_item")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  UOSCManager::AddInt64(message, MenuItem.moduleId);
  UOSCManager::AddInt32(message, MenuItem.menuId);
  UOSCManager::AddInt32(message, MenuItem.index);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::UpdateMenuItemQuantity(const FVCVMenuItem& MenuItem, const float& Value) const {
  UE_LOG(LogTemp, Warning, TEXT("Sending /update_menu_item_quantity %lld"), MenuItem.moduleId);
  FOSCAddress address = UOSCManager::ConvertStringToOSCAddress(FString(TEXT("/update_menu_item_quantity")));
  FOSCMessage message;
  UOSCManager::SetOSCMessageAddress(message, address);

  UOSCManager::AddInt64(message, MenuItem.moduleId);
  UOSCManager::AddInt32(message, MenuItem.menuId);
  UOSCManager::AddInt32(message, MenuItem.index);
  UOSCManager::AddFloat(message, Value);

  OSCClient->SendOSCMessage(message);
}

void AOSCController::AddContextMenuItem(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddContextMenuItem"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;
  
  int menuId;
  UOSCManager::GetInt32(message, 1, menuId);

  int itemIndex;
  UOSCManager::GetInt32(message, 2, itemIndex);
  
  FVCVMenuItem menuItem(moduleId, menuId, itemIndex);

  int32 type;
  UOSCManager::GetInt32(message, 3, type);
  menuItem.type = static_cast<VCVMenuItemType>(type);

  UOSCManager::GetString(message, 4, menuItem.text);
  UOSCManager::GetBool(message, 5, menuItem.checked);
  UOSCManager::GetBool(message, 6, menuItem.disabled);
  UOSCManager::GetFloat(message, 7, menuItem.quantityValue);
  UOSCManager::GetFloat(message, 8, menuItem.quantityMinValue);
  UOSCManager::GetFloat(message, 9, menuItem.quantityMaxValue);
  UOSCManager::GetFloat(message, 10, menuItem.quantityDefaultValue);
  UOSCManager::GetString(message, 11, menuItem.quantityLabel);
  UOSCManager::GetString(message, 12, menuItem.quantityUnit);
  
  OnMenuItemSyncedDelegate.Broadcast(menuItem);
}

void AOSCController::MenuSynced(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("MenuSynced"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;
  
  int menuId;
  UOSCManager::GetInt32(message, 1, menuId);
  
  FVCVMenu menu(moduleId, menuId);

  OnMenuSyncedDelegate.Broadcast(menu);
}


bool AOSCController::ModuleGuard(const FOSCMessage &Message, int64_t &ModuleId) {
  UOSCManager::GetInt64(Message, 0, ModuleId);

  if (!Modules.Contains(ModuleId)) {
    UE_LOG(LogTemp, Warning, TEXT("can't find module %lld"), ModuleId);
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
  GameMode->RegisterSVG(param.svgPaths[0], param.box.size);

  UOSCManager::GetString(message, 29, param.svgPaths[1]);
  GameMode->RegisterSVG(param.svgPaths[1], param.box.size);

  UOSCManager::GetString(message, 30, param.svgPaths[2]);
  GameMode->RegisterSVG(param.svgPaths[2], param.box.size);

  UOSCManager::GetString(message, 31, param.svgPaths[3]);
  GameMode->RegisterSVG(param.svgPaths[3], param.box.size);

  UOSCManager::GetString(message, 32, param.svgPaths[4]);
  GameMode->RegisterSVG(param.svgPaths[4], param.box.size);

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

  AddPort(message, input);
}

void AOSCController::AddOutput(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  // UE_LOG(LogTemp, Warning, TEXT("AddOutput"));

  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;

  int32 outputId;
  UOSCManager::GetInt32(message, 1, outputId);
  Modules[moduleId].Outputs.Add(outputId, VCVPort(outputId, PortType::Output, moduleId));
  VCVPort& output = Modules[moduleId].Outputs[outputId];
  
  AddPort(message, output);
}

void AOSCController::AddPort(const FOSCMessage &Message, VCVPort& vcv_port) {
  UOSCManager::GetString(Message, 2, vcv_port.name);
  UOSCManager::GetString(Message, 3, vcv_port.description);

  UOSCManager::GetFloat(Message, 4, vcv_port.box.pos.x);
  UOSCManager::GetFloat(Message, 5, vcv_port.box.pos.y);
  UOSCManager::GetFloat(Message, 6, vcv_port.box.size.x);
  UOSCManager::GetFloat(Message, 7, vcv_port.box.size.y);
  vcv_port.box *= RENDER_SCALE;

  UOSCManager::GetString(Message, 8, vcv_port.svgPath);

  GameMode->RegisterSVG(vcv_port.svgPath, vcv_port.box.size);

  UOSCManager::GetFloat(Message, 9, vcv_port.bodyColor.R);
  UOSCManager::GetFloat(Message, 10, vcv_port.bodyColor.G);
  UOSCManager::GetFloat(Message, 11, vcv_port.bodyColor.B);
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
  
  VCVCable cable(cableId);

  UOSCManager::GetInt64(message, 1, cable.inputModuleId);
  UOSCManager::GetInt64(message, 2, cable.outputModuleId);
  UOSCManager::GetInt32(message, 3, cable.inputPortId);
  UOSCManager::GetInt32(message, 4, cable.outputPortId);

  // TODO: why
  NotifyReceived("cable", cableId);

  GameMode->QueueCableSpawn(cable);
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

  GameMode->UpdateLight(moduleId, lightId, color);
}

void AOSCController::SyncParam(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  int64_t moduleId;
  if (ModuleGuard(message, moduleId)) return;
  
  int32 paramId;
  UOSCManager::GetInt32(message, 1, paramId);
  VCVParam param(paramId);

  UOSCManager::GetString(message, 2, param.displayValue);
  UOSCManager::GetFloat(message, 3, param.value);
  UOSCManager::GetBool(message, 4, param.visible);
  
  GameMode->UpdateParam(moduleId, param);
}

void AOSCController::SetLibraryJsonPath(const FOSCAddress& AddressPattern, const FOSCMessage &message, const FString &ipaddress, int32 port) {
  FString path;
  UOSCManager::GetString(message, 0, path);

  // UE_LOG(LogTemp, Warning, TEXT("library json path is %s"), *path);
  GameMode->SetLibraryJsonPath(path);
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