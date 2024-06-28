#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVData/VCV.h"

#include "ContextMenu.generated.h"

class Aosc3GameModeBase;
class AVCVModule;
class UContextMenuWidget;
class UWidgetComponent;

UCLASS()
class RACKSIM_API AContextMenu : public AActor {
	GENERATED_BODY()
	
public:	
	AContextMenu();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void Init(const Vec2& ModuleSize);
  void ToggleVisible();

  // add incoming item to menu
  void AddMenuItem(FVCVMenuItem MenuItem);
  // menu fully synced: sort items & SetMenu
  void MenuSynced(FVCVMenu Menu);
  // hide the menu and dump ContextMenus
  void CloseMenu();
  // (re)request menu from rack, potentially creating menu struct
  void MakeMenu(int ParentMenuId = -1, int ParentItemIndex = -1);
  // ask GameMode to ask rack for menu data
  void RequestMenu(int MenuId);

private:
  AVCVModule* Module;
  Aosc3GameModeBase* GameMode;

  USceneComponent* RootSceneComponent;

  UWidgetComponent* ContextMenuWidgetComponent;
  TCHAR* WidgetBlueprintReference =
    TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/BP_ContextMenuWidget.BP_ContextMenuWidget_C'");
  UContextMenuWidget* ContextMenuWidget;
  
  FString MakeMenuBreadcrumbs(int MenuId);

  // set list items from menu struct
  void SetMenu(int MenuId);

  // menu structs
  TArray<FVCVMenu> ContextMenus;
};
