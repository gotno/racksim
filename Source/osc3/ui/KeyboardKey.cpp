#include "UI/KeyboardKey.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"

void UKeyboardKey::NativeConstruct() {
  Super::NativeConstruct();

  Button->OnReleased.AddUniqueDynamic(this, &UKeyboardKey::HandleButtonReleased);
}

void UKeyboardKey::HandleButtonReleased() {
  OnClickedDelegate.Broadcast(this);
}