#pragma once

struct VCVModuleInfo {
  FString Name;
  FString Slug;
  FString Description;
  bool bFavorite;
  TArray<int> Tags;
  
  VCVModuleInfo() {}
  VCVModuleInfo(FString _name, FString _slug)
    : Name(_name), Slug(_slug) {}
  VCVModuleInfo(FString _name, FString _slug, FString _description, bool _favorite)
    : Name(_name), Slug(_slug), Description(_description), bFavorite(_favorite) {}
};

struct VCVPluginInfo {
  FString Name;
  FString Slug;
  
  TMap<FString, VCVModuleInfo> Modules;
  TSet<int> ModuleTags;
  
  VCVPluginInfo() {} 
  VCVPluginInfo(FString _name, FString _slug) : Name(_name), Slug(_slug) {} 
};

struct VCVLibrary {
  TMap<FString, VCVPluginInfo> Plugins;
  TMap<int, FString> TagNames;
};