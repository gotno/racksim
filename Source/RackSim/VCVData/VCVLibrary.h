#pragma once

struct VCVModuleInfo {
  FString Name;
  FString Slug;
  FString Description;
  bool bFavorite;
  TArray<int> Tags;

  VCVModuleInfo() {}
  VCVModuleInfo(FString _slug) : Slug(_slug) {}
};

struct VCVPluginInfo {
  FString Name;
  FString Slug;
  FString Version;

  TMap<FString, VCVModuleInfo> Modules;
  TSet<int> ModuleTags;

  VCVPluginInfo() {} 
  VCVPluginInfo(FString _slug) : Slug(_slug) {} 
};

struct VCVLibrary {
  TMap<FString, VCVPluginInfo> Plugins;
  TMap<int, FString> TagNames;
};