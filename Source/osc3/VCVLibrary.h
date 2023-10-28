#pragma once

struct VCVModuleInfo {
  FString Name;
  FString Slug;
  FString Description;
  TArray<int> Tags;
  
  VCVModuleInfo() {}
  VCVModuleInfo(FString _name, FString _slug)
    : Name(_name), Slug(_slug) {}
  VCVModuleInfo(FString _name, FString _slug, FString _description)
    : Name(_name), Slug(_slug), Description(_description) {}
};

struct VCVPluginInfo {
  FString Name;
  FString Slug;
  
  TMap<FString, VCVModuleInfo> Modules;
  
  VCVPluginInfo() {} 
  VCVPluginInfo(FString _name, FString _slug) : Name(_name), Slug(_slug) {} 
};

struct VCVLibrary {
  TMap<FString, VCVPluginInfo> Plugins;
  TMap<int, FString> TagNames;
};