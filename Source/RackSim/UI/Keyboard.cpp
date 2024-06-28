#include "UI/Keyboard.h"

#include "osc3.h"
#include "MainMenu.h"
#include "UI/KeyboardWidget.h"
#include "UI/KeyboardKey.h"

#include "Components/WidgetComponent.h"

AKeyboard::AKeyboard() {
	PrimaryActorTick.bCanEverTick = true;

  RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Scene Component"));
  SetRootComponent(RootSceneComponent);

  KeyboardWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Keyboard Widget Component"));
  KeyboardWidgetComponent->SetWindowFocusable(false);
  KeyboardWidgetComponent->SetupAttachment(GetRootComponent());

  KeyboardWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
  KeyboardWidgetComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
  KeyboardWidgetComponent->SetCollisionResponseToChannel(WIDGET_TRACE, ECollisionResponse::ECR_Block);

  static ConstructorHelpers::FClassFinder<UKeyboardWidget>
    keyboardWidgetFinder(KeyboardWidgetBlueprintReference);
  if (keyboardWidgetFinder.Succeeded())
    KeyboardWidgetComponent->SetWidgetClass(keyboardWidgetFinder.Class);

  static ConstructorHelpers::FClassFinder<UKeyboardKey>
    keyboardKeyFinder(KeyboardKeyBlueprintReference);
  if (keyboardKeyFinder.Succeeded())
    KeyClass = keyboardKeyFinder.Class;
}

void AKeyboard::BeginPlay() {
	Super::BeginPlay();
	
  KeyboardWidget = Cast<UKeyboardWidget>(KeyboardWidgetComponent->GetUserWidgetObject());
  if (KeyboardWidget) {
    KeyboardWidget->KeyClass = KeyClass;
    KeyboardWidget->Init();
  }

  FVector2D drawSize(933.f, 400.f);
  float desiredWorldHeight = 14.f;
  float scale = desiredWorldHeight / drawSize.Y;

  KeyboardWidgetComponent->SetDrawSize(drawSize);
  KeyboardWidgetComponent->SetWorldScale3D(FVector(1.f, scale, scale));
  KeyboardWidgetComponent->AddWorldOffset(FVector(0.f, 0.f, -desiredWorldHeight * 0.5f));
}

void AKeyboard::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

// TODO: AddUnique?
void AKeyboard::AddOnInputUpdatedDelegate(UObject* Object, const FName& MethodName) {
  if (KeyboardWidget) {
    FScriptDelegate Delegate;
    Delegate.BindUFunction(Object, MethodName);
    KeyboardWidget->OnInputUpdatedDelegate.Add(Delegate);
  }
}

// TODO: AddUnique?
void AKeyboard::AddOnInputConfirmedDelegate(UObject* Object, const FName& MethodName) {
  if (KeyboardWidget) {
    FScriptDelegate Delegate;
    Delegate.BindUFunction(Object, MethodName);
    KeyboardWidget->OnInputConfirmedDelegate.Add(Delegate);
  }
}

void AKeyboard::SetInput(FString Input) {
  if (KeyboardWidget) KeyboardWidget->SetInput(Input);
}

void AKeyboard::ClearInput() {
  if (KeyboardWidget) KeyboardWidget->ClearInput();
}