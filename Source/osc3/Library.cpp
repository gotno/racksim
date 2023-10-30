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

  StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
  StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  StaticMeshComponent->SetupAttachment(GetRootComponent());
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshBody(TEXT("/Script/Engine.StaticMesh'/Game/meshes/unit_widget_base.unit_widget_base'"));
  
  if (MeshBody.Object) StaticMeshComponent->SetStaticMesh(MeshBody.Object);

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
  if (BaseMaterial.Object) {
    BaseMaterialInterface = Cast<UMaterial>(BaseMaterial.Object);
  }

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
  }

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
  // get initial actor bounds, which will be the rendered widget size
  FVector _origin, extent;
  GetActorBounds(false, _origin, extent);
  float scale = DesiredWidth / (extent.Y * 2);

  // scale the base mesh to desired
  float baseWidth = DesiredWidth + (BasePadding * 2);
  StaticMeshComponent->SetWorldScale3D(FVector(1, baseWidth, baseWidth));

  // scale the widget component based on initial bounds
  LibraryWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));
  LibraryWidgetComponent->AddLocalOffset(FVector(0.1f, 0.f, 0.f));
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