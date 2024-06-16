#include "ModuleComponents/ContextMenu.h"

#include "osc3.h"
#include "osc3GameModeBase.h"
#include "OSCController.h"
#include "VCVModule.h"
#include "UI/ContextMenuWidget.h"
#include "UI/ContextMenuEntryData.h"

#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Algo/Reverse.h"

AContextMenu::AContextMenu() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);

  ContextMenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ContextMenuWidget"));
  ContextMenuWidgetComponent->SetWindowFocusable(false);
  ContextMenuWidgetComponent->SetupAttachment(GetRootComponent());

  ContextMenuWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  ContextMenuWidgetComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  ContextMenuWidgetComponent->SetCollisionResponseToChannel(WIDGET_TRACE, ECollisionResponse::ECR_Block);

  static ConstructorHelpers::FClassFinder<UContextMenuWidget>
    contextMenuWidgetObject(WidgetBlueprintReference);
  if (contextMenuWidgetObject.Succeeded()) {
    ContextMenuWidgetComponent->SetWidgetClass(contextMenuWidgetObject.Class);
  }
}

void AContextMenu::BeginPlay() {
	Super::BeginPlay();

  GameMode = Cast<Aosc3GameModeBase>(UGameplayStatics::GetGameMode(this));
  Module = Cast<AVCVModule>(GetOwner());
	
  ContextMenuWidget = Cast<UContextMenuWidget>(ContextMenuWidgetComponent->GetUserWidgetObject());
  ContextMenuWidgetComponent->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
  ContextMenuWidgetComponent->SetVisibility(false);
  
  GameMode->SubscribeMenuItemSyncedDelegate(this);
  GameMode->SubscribeMenuSyncedDelegate(this);
}

void AContextMenu::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AContextMenu::Init(const Vec2& ModuleSize) {
  FVector2D drawSize(350.f, 700.f);
  float desiredContextMenuHeight = ModuleSize.y - 2 * RENDER_SCALE;
  float scale = desiredContextMenuHeight / drawSize.Y;

  ContextMenuWidgetComponent->SetDrawSize(drawSize);
  ContextMenuWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));

  float halfModuleWidth = ModuleSize.x * 0.5f;
  float halfMenuWidth = scale * drawSize.X * 0.5f;

  // TODO: dominant hand
  ContextMenuWidgetComponent->AddWorldOffset(GetActorRightVector() * halfMenuWidth);
  AddActorWorldOffset(GetActorRightVector() * (halfModuleWidth - 0.5f));
  AddActorWorldOffset(-GetActorForwardVector() * (RENDER_SCALE + 1.f));
  // AddActorLocalRotation(FRotator(0.f, 15.f, 0.f));
}

void AContextMenu::AddMenuItem(FVCVMenuItem MenuItem) {
  if (MenuItem.moduleId != Module->Id) return;
  ContextMenus[MenuItem.menuId].MenuItems.Add(MenuItem.index, MenuItem);
}

void AContextMenu::MenuSynced(FVCVMenu Menu) {
  if (Menu.moduleId != Module->Id) return;
  ContextMenus[Menu.id].MenuItems.KeySort([](int A, int B) { return A < B; });
  SetMenu(Menu.id);
}

void AContextMenu::ToggleVisible() {
  if (ContextMenuWidgetComponent->IsVisible()) {
    CloseMenu();
    return;
  }

  // (re)create the base menu struct
  MakeMenu();

  ContextMenuWidgetComponent->SetVisibility(true);
}

void AContextMenu::CloseMenu() {
  ContextMenuWidgetComponent->SetVisibility(false);
  ContextMenus.Empty();
}

void AContextMenu::MakeMenu(int ParentMenuId, int ParentItemIndex) {
  int foundMenuIndex = ContextMenus.IndexOfByPredicate([&](const FVCVMenu& menu) {
    return menu.parentMenuId == ParentMenuId && menu.parentItemIndex == ParentItemIndex;
  });

  if (foundMenuIndex != INDEX_NONE) {
    RequestMenu(foundMenuIndex);
    return;
  }

  int menuId = ContextMenus.Num();
  FVCVMenu menu(Module->Id, menuId);

  menu.parentMenuId = ParentMenuId;
  menu.parentItemIndex = ParentItemIndex;

  ContextMenus.Push(menu);
  RequestMenu(menuId);
}

void AContextMenu::RequestMenu(int MenuId) {
  GameMode->RequestMenu(ContextMenus[MenuId]);
}

FString AContextMenu::MakeMenuBreadcrumbs(int MenuId) {
  TArray<FString> crumbs;

  int currentId = MenuId;
  while (currentId > 0) {
    int parentMenuId = ContextMenus[currentId].parentMenuId;
    int parentItemIndex = ContextMenus[currentId].parentItemIndex;

    crumbs.Push(ContextMenus[parentMenuId].MenuItems[parentItemIndex].text);

    currentId = parentMenuId;
  }

  Algo::Reverse(crumbs);
  return FString::Join(crumbs, _T(" > "));
}

void AContextMenu::SetMenu(int MenuId) {
  TArray<UContextMenuEntryData*> entries;

  // add close/back button
  UContextMenuEntryData* backButtonEntry =
    NewObject<UContextMenuEntryData>(this);
  backButtonEntry->MenuItem.type = VCVMenuItemType::BACK;
  backButtonEntry->MenuItem.text = MenuId == 0 ? FString("Close") : FString("Back");
  backButtonEntry->ContextMenu = this;
  backButtonEntry->ParentMenuId = ContextMenus[MenuId].parentMenuId;
  backButtonEntry->DividerNext = true;
  entries.Add(backButtonEntry);

  // add breadcrumbs label for submenus
  if (MenuId != 0) {
    int parentMenuId = ContextMenus[MenuId].parentMenuId;
    int parentItemIndex = ContextMenus[MenuId].parentItemIndex;

    UContextMenuEntryData* crumbsLabelEntry =
      NewObject<UContextMenuEntryData>(this);
    crumbsLabelEntry->MenuItem.type = VCVMenuItemType::LABEL;
    crumbsLabelEntry->MenuItem.text = MakeMenuBreadcrumbs(MenuId);
    crumbsLabelEntry->DividerNext = true;
    entries.Add(crumbsLabelEntry);
  }

  for (auto& pair : ContextMenus[MenuId].MenuItems) {
    FVCVMenuItem& menuItem = pair.Value;
    VCVMenuItemType divider = VCVMenuItemType::DIVIDER;

    if (menuItem.type == divider) continue;

    UContextMenuEntryData* entry = NewObject<UContextMenuEntryData>(this);
    entry->MenuItem = menuItem;
    entry->ContextMenu = this;

    int& index = pair.Key;
    if (ContextMenus[MenuId].MenuItems.Contains(index + 1)) 
      if (ContextMenus[MenuId].MenuItems[index + 1].type == divider) 
        entry->DividerNext = true;

    entries.Add(entry);
  }

  ContextMenuWidget->SetListItems(entries);
}