#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContextMenuWidget.generated.h"

class UListView;
class UContextMenuEntryData;

UCLASS()
class RACKSIM_API UContextMenuWidget : public UUserWidget {
	GENERATED_BODY()
    
public:
  void SetListItems(TArray<UContextMenuEntryData*> Entries);
    
protected:
  UPROPERTY(meta = (BindWidget))
	UListView* ListView;
};