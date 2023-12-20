#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenu.generated.h"

class UListView;
class UContextMenuEntryData;

UCLASS()
class OSC3_API UContextMenu : public UUserWidget {
	GENERATED_BODY()
    
public:
  void SetListItems(TArray<UContextMenuEntryData*> Entries);
    
protected:
  UPROPERTY(meta = (BindWidget))
	UListView* ListView;
};