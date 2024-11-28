#include "Library.h"

#include "osc3GameModeBase.h"
#include "VCVModule.h"

#include "UI/LibraryWidget.h"
#include "UI/LibraryEntry.h"
#include "UI/FilterListEntryData.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"
#include "Json.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"

ALibrary::ALibrary() {
  PrimaryActorTick.bCanEverTick = true;

  // RootSceneComponent/StaticMeshComponent/OutlineMeshComponent setup in GrabbableActor

  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  if (MeshBody.Object) {
    StaticMeshComponent->SetStaticMesh(MeshBody.Object);
    OutlineMeshComponent->SetStaticMesh(MeshBody.Object);
  }

  PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Preview Mesh"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> PreviewMeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/faced/unit_module_faced.unit_module_faced'"));
  if (PreviewMeshBody.Object) PreviewMeshComponent->SetStaticMesh(PreviewMeshBody.Object);
  PreviewMeshComponent->SetupAttachment(StaticMeshRoot);
  // PreviewMeshComponent->SetHiddenInGame(true);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> PreviewMaterialFinder(
    TEXT("/Script/Engine.Material'/Game/meshes/faced/texture_face_bg.texture_face_bg'")
  );
  if (PreviewMaterialFinder.Object)
    PreviewMaterialInterface = Cast<UMaterial>(PreviewMaterialFinder.Object);

  // loading indicator material
  static ConstructorHelpers::FObjectFinder<UMaterial> LoadingMaterialFinder(
    TEXT("/Script/Engine.Material'/Game/materials/loading.loading'")
  );
  if (LoadingMaterialFinder.Object)
    LoadingMaterialInterface = Cast<UMaterial>(LoadingMaterialFinder.Object);

  // OutlineMaterialInterface setup in GrabbableActor

  LibraryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LibraryWidget"));
  LibraryWidgetComponent->SetupAttachment(StaticMeshComponent);
  LibraryWidgetComponent->AddWorldOffset(LibraryWidgetComponent->GetForwardVector() * -0.01f);
  LibraryWidgetComponent->SetWindowFocusable(false);

  LibraryWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  LibraryWidgetComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  LibraryWidgetComponent->SetCollisionResponseToChannel(WIDGET_TRACE, ECollisionResponse::ECR_Block);

  static ConstructorHelpers::FClassFinder<ULibraryWidget> libraryWidgetObject(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_LibraryWidget.BP_LibraryWidget_C'"));
  if (libraryWidgetObject.Succeeded()) {
    LibraryWidgetComponent->SetWidgetClass(libraryWidgetObject.Class);
  }
}

void ALibrary::BeginPlay() {
  Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
    PreviewMeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Black);
  }

  if (PreviewMaterialInterface) {
    PreviewMaterialInstance = UMaterialInstanceDynamic::Create(PreviewMaterialInterface, this);
  }

  if (LoadingMaterialInterface) {
    LoadingMaterialInstance = UMaterialInstanceDynamic::Create(LoadingMaterialInterface, this);
  }

  LibraryWidget = Cast<ULibraryWidget>(LibraryWidgetComponent->GetUserWidgetObject());
  LibraryWidgetComponent->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
  LibraryWidgetComponent->AddWorldOffset(StaticMeshComponent->GetForwardVector() * -0.01f);

  Tags.Add(TAG_INTERACTABLE);

  SetScale();
}

void ALibrary::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
}

void ALibrary::SetJsonPath(FString& JsonPath) {
  FString JsonStr;
	if (FFileHelper::LoadFileToString(JsonStr, *JsonPath)) {
    ParseLibraryJson(JsonStr);
    Refresh();

    IFileManager& FileManager = IFileManager::Get();
    FileManager.Delete(*JsonPath);
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Unable to load library json file"));
  }
}

void ALibrary::ParseLibraryJson(FString& JsonStr) {
  TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(*JsonStr);
  TSharedPtr<FJsonValue> OutJson;

  if (FJsonSerializer::Deserialize(JsonReader, OutJson)) {
    TSharedPtr<FJsonObject> rootJ = OutJson->AsObject();
    TSharedPtr<FJsonObject> pluginsJ = rootJ->GetObjectField(FString("plugins"));
    for (auto& pluginPair : pluginsJ->Values) {
      // UE_LOG(LogTemp, Warning, TEXT("ParseLibraryJson\tplugin: %s"), *pluginPair.Key);
      TSharedPtr<FJsonObject> pluginJ = pluginPair.Value->AsObject();

      VCVPluginInfo& plugin = Model.Plugins.Emplace(pluginPair.Key, pluginPair.Key);
      plugin.Name = pluginJ->GetStringField(FString("name"));
      plugin.Version = pluginJ->GetStringField(FString("version"));

      for (auto& modulePair : pluginJ->GetObjectField("modules")->Values) {
        // UE_LOG(LogTemp, Warning, TEXT("ParseLibraryJson\t  module: %s"), *modulePair.Key);
        TSharedPtr<FJsonObject> moduleJ = modulePair.Value->AsObject();

        VCVModuleInfo& module = plugin.Modules.Emplace(modulePair.Key, modulePair.Key);
        module.Name = moduleJ->GetStringField(FString("name")); 
        module.Description = moduleJ->GetStringField(FString("description")); 
        module.bFavorite = moduleJ->GetBoolField(FString("bFavorite")); 
        for (auto& tagId : moduleJ->GetArrayField(FString("tagIds")))
          module.Tags.Add(tagId->AsNumber());
      }
    }

    TSharedPtr<FJsonObject> tagNamesJ = rootJ->GetObjectField(FString("tagNames"));
    for (auto& tagNamePair : tagNamesJ->Values) {
      // UE_LOG(LogTemp, Warning, TEXT("ParseLibraryJson\ttagName: %d- %s"), FCString::Atoi(*tagNamePair.Key), *tagNamePair.Value->AsString());
      Model.TagNames.Add(FCString::Atoi(*tagNamePair.Key), tagNamePair.Value->AsString());
    }
  } else {
    UE_LOG(LogTemp, Warning, TEXT("trouble deserializing library json"));
  }
}

void ALibrary::Refresh() {
  RefreshLibraryList();
  RefreshBrandFilterList();
  RefreshTagsFilterList();
}

void ALibrary::RefreshLibraryList() {
  LibraryWidget->SetLibraryListItems(GenerateLibraryEntries());
}

void ALibrary::RefreshBrandFilterList() {
  LibraryWidget->SetBrandFilterListItems(GenerateBrandFilterEntries());
}

void ALibrary::RefreshTagsFilterList() {
  LibraryWidget->SetTagsFilterListItems(GenerateTagsFilterEntries());
}

void ALibrary::SetScale() {
  float padding = 0.2f * DEFAULT_RENDER_SCALE;
  float desiredHeight = UNSCALED_MODULE_HEIGHT * DEFAULT_RENDER_SCALE;
  float desiredMenuHeight = desiredHeight - (2 * padding);

  StaticMeshComponent->SetWorldScale3D(
    FVector(
      DEFAULT_RENDER_SCALE * UNSCALED_MODULE_DEPTH,
      desiredHeight,
      desiredHeight
    )
  );
  OutlineMeshComponent->SetWorldScale3D(
    FVector(
      DEFAULT_RENDER_SCALE * UNSCALED_MODULE_DEPTH,
      desiredHeight,
      desiredHeight
    )
  );

  FVector2D drawSize(700.f, 700.f);
  float scale = desiredMenuHeight / drawSize.Y;
  LibraryWidgetComponent->SetDrawSize(drawSize);
  LibraryWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));
}

void ALibrary::SetPreviewsPath(FString& inPreviewsPath) {
  PreviewsPath = inPreviewsPath;

  PreviewVersionsJsonPath = PreviewsPath;
  PreviewVersionsJsonPath.Append(TEXT("screenshot_plugin_versions.json"));
  ParsePreviewVersionsJson();
}

void ALibrary::ParsePreviewVersionsJson() {
  FString JsonStr;
  if (!FFileHelper::LoadFileToString(JsonStr, *PreviewVersionsJsonPath)) return;

  TSharedRef<TJsonReader<TCHAR>> JsonReader =
    TJsonReaderFactory<TCHAR>::Create(*JsonStr);
  TSharedPtr<FJsonValue> OutJson;

  if (!FJsonSerializer::Deserialize(JsonReader, OutJson)) return;

  TSharedPtr<FJsonObject> rootJ = OutJson->AsObject();
  for (auto& versionPair : rootJ->Values) {
    Model.PreviewVersions.Add(versionPair.Key, versionPair.Value->AsString());
  }
}

void ALibrary::CheckPluginVersionsForPreviewUpdates() {
  if (bCheckedPreviews) return;
  bCheckedPreviews = true;

  if (!FPaths::FileExists(*PreviewVersionsJsonPath)) {
    GameMode->AlertGeneratePreviews();
    return;
  }

  TArray<FString> previewUpdatePluginSlugs;
  FString previewVersion;

  // check for updated plugins
  for (auto& pluginPair : Model.Plugins) {
    VCVPluginInfo& plugin = pluginPair.Value;

    if (Model.PreviewVersions.Contains(plugin.Slug)) {
      previewVersion = Model.PreviewVersions[plugin.Slug];
      if (!previewVersion.Equals(plugin.Version))
        previewUpdatePluginSlugs.Add(plugin.Slug);
    }
  }

  // check for new plugins
  bool newPlugins = Model.Plugins.Num() > Model.PreviewVersions.Num();

  if (newPlugins || previewUpdatePluginSlugs.Num() > 0) {
    GameMode->AlertGeneratePreviews(previewUpdatePluginSlugs);
  }
}

void ALibrary::ShowPreview(FString& PluginSlug, FString& ModuleSlug) {
  CheckPluginVersionsForPreviewUpdates();

  PreviewMeshComponent->SetMaterial(1, LoadingMaterialInstance);
  PreviewMeshComponent->SetHiddenInGame(false);

  FString texturePath = PreviewsPath;
  texturePath.Appendf(TEXT("%s/%s.png"), *PluginSlug, *ModuleSlug);
  UTexture2D* Texture =
    UKismetRenderingLibrary::ImportFileAsTexture2D(this, texturePath);
  if (!Texture) {
    HidePreview();
    return;
  }
  PreviewMeshComponent->SetMaterial(1, PreviewMaterialInstance);
  PreviewMaterialInstance->SetTextureParameterValue(FName("texture"), Texture);

  FVector previewScale =
    FVector(
      AVCVModule::Scale * UNSCALED_MODULE_DEPTH,
      1.f,
      AVCVModule::Scale * UNSCALED_MODULE_HEIGHT
    );

  // height scaled by texture aspect ratio
  float previewPanelWidth =
    previewScale.Z * Texture->GetSurfaceWidth() / Texture->GetSurfaceHeight();
  previewScale.Y = previewPanelWidth;
  PreviewMeshComponent->SetWorldScale3D(previewScale);

  FVector previewLocation;
  FRotator previewRotation;
  GetModuleLandingPosition(previewScale.Y, previewLocation, previewRotation);
  PreviewMeshComponent->SetWorldLocation(previewLocation);
  PreviewMeshComponent->SetWorldRotation(previewRotation);

  UpdateLot(previewPanelWidth, true);
}

void ALibrary::HidePreview() {
  PreviewMeshComponent->SetHiddenInGame(true);
  UpdateLot(true);
}

void ALibrary::ParkModule(AVCVModule* Module) {
  Module->OnFirstMoved.AddUObject(this, &ALibrary::UnparkModule);
  Module->AttachToComponent(StaticMeshRoot, FAttachmentTransformRules::KeepWorldTransform);

  TArray<AVCVModule*> newParkedModules;
  newParkedModules.Add(Module);
  for (auto& ParkedModule : ParkedModules) newParkedModules.Add(ParkedModule);

  ParkedModules = newParkedModules;
  UpdateLot(true);
}

void ALibrary::UnparkModule(AVCVModule* Module) {
  Module->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

  ParkedModules = ParkedModules.FilterByPredicate(
    [Module](AVCVModule* ParkedModule) {
      return ParkedModule != Module;
    }
  );
  UpdateLot();
}

void ALibrary::UpdateLot(bool bInstant) {
  UpdateLot(0.f, bInstant);
}

void ALibrary::UpdateLot(float PreviewPanelWidth, bool bInstant) {
  if (ParkedModules.Num() == 0) return;

  FVector parkingLocation;
  FRotator parkingRotation;

  GetModuleLandingPosition(
    ParkedModules[0]->GetPanelWidth(),
    parkingLocation,
    parkingRotation
  );
  if (PreviewPanelWidth > 0.f) {
    parkingLocation +=
      StaticMeshComponent->GetRightVector() * (PreviewPanelWidth + ParkingLotGutter);
  }
  ParkedModules[0]->Summon(parkingLocation, parkingRotation, bInstant);

  if (ParkedModules.Num() == 1) return;

  for (int i = 1; i < ParkedModules.Num(); i++) {
    float distanceToNextPosition = (
      ParkedModules[i - 1]->GetPanelWidth() * 0.5f +
      ParkingLotGutter +
      ParkedModules[i]->GetPanelWidth() * 0.5f
    );
    parkingLocation += StaticMeshComponent->GetRightVector() * distanceToNextPosition;

    ParkedModules[i]->Summon(parkingLocation, parkingRotation, bInstant);
  }
}

TArray<ULibraryEntry*> ALibrary::GenerateLibraryEntries() {
  TArray<ULibraryEntry*> entries;
  
  for (auto& pluginPair : Model.Plugins) {
    VCVPluginInfo& pluginInfo = pluginPair.Value;
    
    if (!BrandFilter.IsEmpty() && !BrandFilter.Equals(pluginInfo.Name)) continue;

    for (auto& modulePair : pluginInfo.Modules) {
      VCVModuleInfo& moduleInfo = modulePair.Value;

      if (bFavoritesFilterActive && !moduleInfo.bFavorite) continue;
      if (!TagFilters.IsEmpty()) {
        bool moduleMatchesTags{true};
        for (int& tagId : TagFilters) {
          moduleMatchesTags = moduleMatchesTags && moduleInfo.Tags.Contains(tagId); 
        }
        if (!moduleMatchesTags) continue;
      }

      ULibraryEntry* entry = NewObject<ULibraryEntry>(this);

      entry->PluginName = pluginInfo.Name;
      entry->PluginSlug = pluginInfo.Slug;
      entry->ModuleDescription = moduleInfo.Description;
      entry->ModuleSlug = moduleInfo.Slug;
      entry->ModuleName = moduleInfo.Name;
      entry->bFavorite = moduleInfo.bFavorite;
      
      TArray<FString> tagNames;
      for (int& tagId : moduleInfo.Tags) {
        if (Model.TagNames.Contains(tagId)) tagNames.Add(Model.TagNames[tagId]);
      }
      tagNames.Sort();
      entry->Tags = FString::Join(tagNames, _T(", "));

      entries.Add(entry);
    }
  }

  entries.Sort([](const ULibraryEntry& a, const ULibraryEntry& b) {
    if (a.PluginName < b.PluginName) return true;
    if (a.PluginName > b.PluginName) return false;
    if (a.ModuleName < b.ModuleName) return true;
    if (a.ModuleName > b.ModuleName) return false;
    return false;
  });

  return entries;
}

TArray<UFilterListEntryData*> ALibrary::GenerateBrandFilterEntries() {
  TArray<UFilterListEntryData*> brandFilterEntries;
  TArray<FString> addedBrands;

  for (auto& pluginPair : Model.Plugins) {
    VCVPluginInfo& pluginInfo = pluginPair.Value;

    // VCV has two sets of plugins (core, fundamental) under one name
    if (addedBrands.Contains(pluginInfo.Name)) continue;
    addedBrands.Add(pluginInfo.Name);
    
    UFilterListEntryData* entry = NewObject<UFilterListEntryData>(this);

    entry->Label = pluginInfo.Name;
    entry->StringValue = pluginInfo.Name;
    entry->Type = EFilterType::BRAND;

    if (BrandFilter.Equals(entry->StringValue)) entry->bSelected = true;

    brandFilterEntries.Add(entry);
  }

  brandFilterEntries.Sort([](const UFilterListEntryData& a, const UFilterListEntryData& b) {
    if (a.Label < b.Label) return true;
    return false;
  });
  
  return brandFilterEntries;
}

TArray<UFilterListEntryData*> ALibrary::GenerateTagsFilterEntries() {
  TArray<UFilterListEntryData*> tagsFilterEntries;

  for (auto& tagPair : Model.TagNames) {
    FString& tagName = tagPair.Value;
    int& tagIndex = tagPair.Key;

    UFilterListEntryData* entry = NewObject<UFilterListEntryData>(this);
    entry->Label = tagName;
    entry->IntValue = tagIndex;
    entry->Type = EFilterType::TAGS;

    if (TagFilters.Contains(entry->IntValue)) entry->bSelected = true;

    tagsFilterEntries.Add(entry);
  }

  tagsFilterEntries.Sort([](const UFilterListEntryData& a, const UFilterListEntryData& b) {
    if (a.Label < b.Label) return true;
    return false;
  });
  
  return tagsFilterEntries;
}

void ALibrary::AddFilter(UFilterListEntryData* FilterListEntryData) {
  switch (FilterListEntryData->Type) {
    case EFilterType::BRAND:
      BrandFilter = FilterListEntryData->StringValue;
      LibraryWidget->SetBrandFilterButtonLabel(BrandFilter, true);
      RefreshBrandFilterList();
      RefreshLibraryList();
      break;
    case EFilterType::TAGS:
      TagFilters.Add(FilterListEntryData->IntValue);
      {
        TArray<FString> tagNames;
        for (int& tagId : TagFilters) {
          if (Model.TagNames.Contains(tagId)) tagNames.Add(Model.TagNames[tagId]);
        }
        tagNames.Sort();
        LibraryWidget->SetTagsFilterButtonLabel(FString::Join(tagNames, _T(", ")), true);
      }

      RefreshTagsFilterList();
      RefreshLibraryList();
      break;
    default:
      break;
  }
}

void ALibrary::RemoveFilter(UFilterListEntryData* FilterListEntryData) {
  switch (FilterListEntryData->Type) {
    case EFilterType::BRAND:
      BrandFilter.Empty();
      LibraryWidget->SetBrandFilterButtonLabel(FString("All Brands"), false);
      RefreshBrandFilterList();
      RefreshLibraryList();
      break;
    case EFilterType::TAGS:
      TagFilters.Remove(FilterListEntryData->IntValue);
      if (TagFilters.IsEmpty()) {
        LibraryWidget->SetTagsFilterButtonLabel(FString("All Tags"), false);
      } else {
        TArray<FString> tagNames;
        for (int& tagId : TagFilters) {
          if (Model.TagNames.Contains(tagId)) tagNames.Add(Model.TagNames[tagId]);
        }
        tagNames.Sort();
        LibraryWidget->SetTagsFilterButtonLabel(FString::Join(tagNames, _T(", ")), true);
      }
      RefreshTagsFilterList();
      RefreshLibraryList();
      break;
    default:
      break;
  }
}

void ALibrary::SetFavoritesFilterActive(bool bActive) {
  bFavoritesFilterActive = bActive;
  RefreshLibraryList();
}

void ALibrary::SetModuleFavorite(FString PluginSlug, FString ModuleSlug, bool bFavorite) {
  Model.Plugins[PluginSlug].Modules[ModuleSlug].bFavorite = bFavorite;
  RefreshLibraryList();
}

void ALibrary::ClearBrandFilter() {
  BrandFilter.Empty();
  LibraryWidget->SetBrandFilterButtonLabel(FString("All Brands"), false);
  RefreshBrandFilterList();
  RefreshLibraryList();
}

void ALibrary::ClearTagsFilter() {
  TagFilters.Empty();
  LibraryWidget->SetTagsFilterButtonLabel(FString("All Tags"), false);
  RefreshTagsFilterList();
  RefreshLibraryList();
}

void ALibrary::GetPosition(FVector& Location, FRotator& Rotation) {
  Location = StaticMeshComponent->GetComponentLocation();
  Rotation = StaticMeshComponent->GetComponentRotation();
}

void ALibrary::GetModuleLandingPosition(const float& ModuleWidth, FVector& Location, FRotator& Rotation) {
  float toEdge = DesiredWidth * 0.5;
  float moduleOffset = ModuleWidth * 0.5f;

  GetPosition(Location, Rotation);

  // TODO: dominant hand
  Location += StaticMeshComponent->GetRightVector() * (toEdge + moduleOffset);
}