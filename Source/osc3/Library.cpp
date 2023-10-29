#include "Library.h"

#include "LibraryWidget.h"
#include "LibraryEntry.h"

#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"

ALibrary::ALibrary() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
  SetRootComponent(RootSceneComponent);

  LibraryWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LibraryWidget"));
  LibraryWidgetComponent->SetupAttachment(GetRootComponent());
  LibraryWidgetComponent->SetWindowFocusable(false);

  static ConstructorHelpers::FClassFinder<ULibraryWidget> libraryWidgetObject(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_LibraryWidget.BP_LibraryWidget_C'"));
  if (libraryWidgetObject.Succeeded()) {
    LibraryWidgetComponent->SetWidgetClass(libraryWidgetObject.Class);
  }
}

void ALibrary::BeginPlay() {
	Super::BeginPlay();
  LibraryWidget = Cast<ULibraryWidget>(LibraryWidgetComponent->GetUserWidgetObject());
}

void ALibrary::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void ALibrary::Init(VCVLibrary vcv_library) {
  Model = vcv_library;
  SetScale();
  LibraryWidget->SetListItems(GenerateEntries());
}

void ALibrary::SetScale() {
  FVector _origin, extent;
  GetActorBounds(false, _origin, extent);
  float scale = DesiredWidth / extent.Y;
  SetActorScale3D(FVector(1.f, scale, scale));
}

TArray<ULibraryEntry*> ALibrary::GenerateEntries() {
  TArray<ULibraryEntry*> entries;
  
  for (auto& pluginPair : Model.Plugins) {
    VCVPluginInfo& pluginInfo = pluginPair.Value;
    for (auto& modulePair : pluginInfo.Modules) {
      VCVModuleInfo& moduleInfo = modulePair.Value;

      ULibraryEntry* entry = NewObject<ULibraryEntry>(this);

      entry->PluginName = pluginInfo.Name;
      entry->PluginSlug = pluginInfo.Slug;
      entry->ModuleName = moduleInfo.Name;
      entry->ModuleDescription = moduleInfo.Description;
      entry->ModuleSlug = moduleInfo.Slug;
      for (int& tagId : moduleInfo.Tags) {
        if (Model.TagNames.Contains(tagId)) entry->Tags.Add(Model.TagNames[tagId]);
      }
      entries.Add(entry);
    }
  }
  return entries;
}