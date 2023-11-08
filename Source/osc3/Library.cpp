#include "Library.h"

#include "LibraryWidget.h"
#include "LibraryEntry.h"

#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

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

  static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterial(TEXT("/Script/Engine.Material'/Game/meshes/faced/generic_base.generic_base'"));
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
  }

  if (OutlineMaterialInterface) {
    OutlineMaterialInstance = UMaterialInstanceDynamic::Create(OutlineMaterialInterface, this);
    OutlineMeshComponent->SetMaterial(0, OutlineMaterialInstance);
  }

  LibraryWidget = Cast<ULibraryWidget>(LibraryWidgetComponent->GetUserWidgetObject());

  Tags.Add(TAG_INTERACTABLE);
  Tags.Add(TAG_GRABBABLE);
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
  StaticMeshComponent->SetWorldScale3D(FVector(RENDER_SCALE * MODULE_DEPTH, baseWidth, baseWidth));

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

  entries.Sort([](const ULibraryEntry& a, const ULibraryEntry& b) {
    if (a.PluginSlug < b.PluginSlug) return true;
    if (a.PluginSlug > b.PluginSlug) return false;
    if (a.ModuleName < b.ModuleName) return true;
    if (a.ModuleName > b.ModuleName) return false;
    return false;
  });

  return entries;
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