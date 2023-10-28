#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "VCVLibrary.h"

#include "Library.generated.h"

class USceneComponent;
class UWidgetComponent;
class ULibraryWidget;
class ULibraryEntryWidget;
class ULibraryEntry;

UCLASS()
class OSC3_API ALibrary : public AActor {
	GENERATED_BODY()
	
public:	
	ALibrary();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
  void Init(VCVLibrary model);

private:
  UPROPERTY(VisibleAnywhere)
  USceneComponent* RootSceneComponent;
  
  UPROPERTY(EditDefaultsOnly)
  UWidgetComponent* LibraryWidgetComponent;
  ULibraryWidget* LibraryWidget;
  
  TArray<ULibraryEntry*> GenerateEntries();
  
  VCVLibrary Model;
};