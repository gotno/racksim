#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class UWidgetComponent;
class UKeyboardWidget;

#include "Keyboard.generated.h"

UCLASS()
class OSC3_API AKeyboard : public AActor {
	GENERATED_BODY()
	
public:	
	AKeyboard();
  void SetInput(FString Input);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

  void ClearInput();
  void AddOnInputUpdatedDelegate(UObject* Object, const FName& MethodName);
  void AddOnInputConfirmedDelegate(UObject* Object, const FName& MethodName);

private:
  USceneComponent* RootSceneComponent;

  UWidgetComponent* KeyboardWidgetComponent;
  TCHAR* KeyboardWidgetBlueprintReference =
    TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/keyboard/WBP_Keyboard.WBP_Keyboard_C'");
  UKeyboardWidget* KeyboardWidget;

  TCHAR* KeyboardKeyBlueprintReference =
    TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/widgets/keyboard/WBP_KeyboardKey.WBP_KeyboardKey_C'");

  TSubclassOf<class UUserWidget> KeyClass;
};
