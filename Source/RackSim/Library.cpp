#include "Library.h"

#include "UI/LibraryWidget.h"
#include "UI/LibraryEntry.h"
#include "UI/FilterListEntryData.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "Json.h"

ALibrary::ALibrary() {
	PrimaryActorTick.bCanEverTick = true;

  // RootSceneComponent/StaticMeshComponent/OutlineMeshComponent setup in GrabbableActor
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_module.unit_module'"));
  
  if (MeshBody.Object) {
    StaticMeshComponent->SetStaticMesh(MeshBody.Object);
    OutlineMeshComponent->SetStaticMesh(MeshBody.Object);
  }

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/materials/generic_color.generic_color'"));
  if (BaseMaterial.Object) BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);

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

  if (BaseMaterialInterface) {
    BaseMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterialInterface, this);
    StaticMeshComponent->SetMaterial(0, BaseMaterialInstance);
    BaseMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor::Black);
  }

  LibraryWidget = Cast<ULibraryWidget>(LibraryWidgetComponent->GetUserWidgetObject());
  LibraryWidgetComponent->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
  LibraryWidgetComponent->AddWorldOffset(StaticMeshComponent->GetForwardVector() * -0.01f);

  Tags.Add(TAG_INTERACTABLE);

  SetScale();
}

void ALibrary::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

  if (bSummoned) {
    SummonAlpha += DeltaTime / 0.4f; // seconds to finish

    SetActorLocation(
      FMath::InterpEaseOut<FVector>(
        StartLocation, TargetLocation, SummonAlpha, 4.f
      )
    );
    SetActorRotation(
      FMath::InterpEaseOut<FRotator>(
        StartRotation, TargetRotation, SummonAlpha, 4.f
      )
    );

    if (SummonAlpha >= 1.f) {
      SummonAlpha = 0.f;
      bSummoned = false;
    }
  }
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

void ALibrary::Summon(FVector Location, FRotator Rotation) {
  StartLocation = GetActorLocation();
  StartRotation = GetActorRotation();
  TargetLocation = Location;
  TargetRotation = Rotation;
  SetActorHiddenInGame(false);
  bSummoned = true;
}