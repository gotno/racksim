#include "Library.h"

#include "ui/LibraryWidget.h"
#include "ui/LibraryEntry.h"
#include "ui/BasicListEntryData.h"

#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Json.h"

ALibrary::ALibrary() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
  SetRootComponent(RootSceneComponent);

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetupAttachment(GetRootComponent());
  StaticMeshComponent->SetRenderInDepthPass(true);
  StaticMeshComponent->SetRenderCustomDepth(true);
  StaticMeshComponent->SetCustomDepthStencilValue(2);

  OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Outline Mesh"));
  OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  OutlineMeshComponent->SetupAttachment(StaticMeshComponent);
  OutlineMeshComponent->SetVisibility(false);
  OutlineMeshComponent->SetWorldScale3D(FVector(1.1f, 1.1f, 1.1f));
  OutlineMeshComponent->AddLocalOffset(FVector(-0.05f, 0.f, 0.f));
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_widget_base.unit_widget_base'"));
  
  if (MeshBody.Object) {
    StaticMeshComponent->SetStaticMesh(MeshBody.Object);
    OutlineMeshComponent->SetStaticMesh(MeshBody.Object);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> OutlineMaterial(TEXT("/Script/Engine.Material'/Game/materials/looman_outlines/M_LocalOutlines.M_LocalOutlines'"));
  if (OutlineMaterial.Object) OutlineMaterialInterface = Cast<UMaterial>(OutlineMaterial.Object);

  LibraryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LibraryWidget"));
  LibraryWidgetComponent->SetupAttachment(StaticMeshComponent);
  LibraryWidgetComponent->SetWindowFocusable(false);

  static ConstructorHelpers::FClassFinder<ULibraryWidget> libraryWidgetObject(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_LibraryWidget.BP_LibraryWidget_C'"));
  if (libraryWidgetObject.Succeeded()) {
    LibraryWidgetComponent->SetWidgetClass(libraryWidgetObject.Class);
  }
}

void ALibrary::BeginPlay() {
	Super::BeginPlay();

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Black);
  }

  if (OutlineMaterialInterface) {
    OutlineMaterialInstance = UMaterialInstanceDynamic::Create(OutlineMaterialInterface, this);
    OutlineMeshComponent->SetMaterial(0, OutlineMaterialInstance);
  }

  LibraryWidget = Cast<ULibraryWidget>(LibraryWidgetComponent->GetUserWidgetObject());

  Tags.Add(TAG_INTERACTABLE);
  Tags.Add(TAG_GRABBABLE);

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
  // get initial actor bounds, which will be the rendered widget size
  FVector _origin, extent;
  GetActorBounds(false, _origin, extent);
  float scale = DesiredWidth / (extent.Y * 2);

  // scale the base mesh to desired
  float baseWidth = DesiredWidth + (BasePadding * 2);
  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, baseWidth, baseWidth));

  // scale the widget component based on initial bounds
  LibraryWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));
  LibraryWidgetComponent->AddLocalOffset(FVector(0.1f, 0.f, 0.f));
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

TArray<UBasicListEntryData*> ALibrary::GenerateBrandFilterEntries() {
  TArray<UBasicListEntryData*> brandFilterEntries;
  TArray<FString> addedBrands;

  for (auto& pluginPair : Model.Plugins) {
    VCVPluginInfo& pluginInfo = pluginPair.Value;

    // VCV has two sets of plugins (core, fundamental) under one name
    if (addedBrands.Contains(pluginInfo.Name)) continue;
    addedBrands.Add(pluginInfo.Name);
    
    UBasicListEntryData* entry = NewObject<UBasicListEntryData>(this);

    entry->Label = pluginInfo.Name;
    entry->StringValue = pluginInfo.Name;
    entry->Type = BasicListEntryType::BRAND_FILTER;

    if (BrandFilter.Equals(entry->StringValue)) entry->bSelected = true;

    brandFilterEntries.Add(entry);
  }

  brandFilterEntries.Sort([](const UBasicListEntryData& a, const UBasicListEntryData& b) {
    if (a.Label < b.Label) return true;
    return false;
  });
  
  return brandFilterEntries;
}

TArray<UBasicListEntryData*> ALibrary::GenerateTagsFilterEntries() {
  TArray<UBasicListEntryData*> tagsFilterEntries;

  for (auto& tagPair : Model.TagNames) {
    FString& tagName = tagPair.Value;
    int& tagIndex = tagPair.Key;

    UBasicListEntryData* entry = NewObject<UBasicListEntryData>(this);
    entry->Label = tagName;
    entry->IntValue = tagIndex;
    entry->Type = BasicListEntryType::TAGS_FILTER;

    if (TagFilters.Contains(entry->IntValue)) entry->bSelected = true;

    tagsFilterEntries.Add(entry);
  }

  tagsFilterEntries.Sort([](const UBasicListEntryData& a, const UBasicListEntryData& b) {
    if (a.Label < b.Label) return true;
    return false;
  });
  
  return tagsFilterEntries;
}

void ALibrary::AddFilter(UBasicListEntryData* BasicListEntryData) {
  switch (BasicListEntryData->Type) {
    case BasicListEntryType::BRAND_FILTER:
      BrandFilter = BasicListEntryData->StringValue;
      LibraryWidget->SetBrandFilterButtonLabel(BrandFilter, true);
      RefreshBrandFilterList();
      RefreshLibraryList();
      break;
    case BasicListEntryType::TAGS_FILTER:
      TagFilters.Add(BasicListEntryData->IntValue);
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

void ALibrary::RemoveFilter(UBasicListEntryData* BasicListEntryData) {
  switch (BasicListEntryData->Type) {
    case BasicListEntryType::BRAND_FILTER:
      BrandFilter.Empty();
      LibraryWidget->SetBrandFilterButtonLabel(FString("All Brands"), false);
      RefreshBrandFilterList();
      RefreshLibraryList();
      break;
    case BasicListEntryType::TAGS_FILTER:
      TagFilters.Remove(BasicListEntryData->IntValue);
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

void ALibrary::SetHighlighted(bool bHighlighted, FLinearColor HighlightColor) {
  OutlineMeshComponent->SetVisibility(bHighlighted);
  OutlineMaterialInstance->SetVectorParameterValue(FName("Color"), HighlightColor);
}

void ALibrary::EngageGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab engage"), *GetActorNameOrLabel());
  bGrabEngaged = true;
  GrabOffset = GrabbedLocation - GetActorLocation();
  StaticMeshComponent->AddWorldOffset(-GrabOffset);

  LastGrabbedRotation = GrabbedRotation;
  LastGrabbedLocation = GrabbedLocation - GrabOffset;
  LastLocationDelta = FVector(0.f, 0.f, 0.f);
  SetHighlighted(false);
}

void ALibrary::AlterGrab(FVector GrabbedLocation, FRotator GrabbedRotation) {
  FQuat qFrom = LastGrabbedRotation.Quaternion();
  FQuat qTo =  GrabbedRotation.Quaternion();
  FQuat qDelta = qTo * qFrom.Inverse();
  
  FVector locationDelta = GrabbedLocation - LastGrabbedLocation;
  locationDelta = UKismetMathLibrary::WeightedMovingAverage_FVector(
    locationDelta,
    LastLocationDelta,
    0.4f
  );
  LastLocationDelta = locationDelta;

  AddActorWorldOffset(locationDelta);
  AddActorWorldRotation(qDelta);

  LastGrabbedLocation = GrabbedLocation;
  LastGrabbedRotation = GrabbedRotation;
}

void ALibrary::ReleaseGrab() {
  // UE_LOG(LogTemp, Warning, TEXT("%s: grab release"), *GetActorNameOrLabel());
  bGrabEngaged = false;
  StaticMeshComponent->AddWorldOffset(GrabOffset);
  AddActorWorldOffset(-GrabOffset);
}