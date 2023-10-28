#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LibraryWidget.generated.h"

class UListView;
class ULibraryEntry;

UCLASS()
class OSC3_API ULibraryWidget : public UUserWidget {
	GENERATED_BODY()

public:
  void SetListItems(TArray<ULibraryEntry*> Entries);

protected:
	virtual void NativeConstruct() override;	
  
  UPROPERTY(meta = (BindWidget))
	UListView* LibraryListView;
};