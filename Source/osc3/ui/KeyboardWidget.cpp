#include "UI/KeyboardWidget.h"

#include "osc3.h"
#include "UI/KeyboardKey.h"

#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

void UKeyboardWidget::NativeConstruct() {
  Super::NativeConstruct();
}

void UKeyboardWidget::Init() {
  MakeKeys();
}

void UKeyboardWidget::HandleKeyReleased(UKeyboardKey* Key) {
  if (Key->Data.Type == EKeyType::CONFIRM) {
    OnInputConfirmedDelegate.Broadcast(InputText);
    return;
  }

  switch (Key->Data.Type) {
    case EKeyType::RAW:
      InputText.InsertAt(
        CursorIndex,
        bShifted ? Key->Data.SecondaryLabel : Key->Data.PrimaryLabel
      );
      CursorIndex++;
      break;
    case EKeyType::BACKSPACE:
      if (CursorIndex > 0) InputText.RemoveAt(--CursorIndex, 1);
      break;
    case EKeyType::SHIFT:
      bShifted = !bShifted;
      bShifted ? SetShifted() : SetUnshifted();
      break;
    case EKeyType::SPACE:
      InputText.InsertAt(CursorIndex, FString(" "));
      CursorIndex++;
      break;
    case EKeyType::HOME:
      CursorIndex = 0;
      break;
    case EKeyType::LEFT:
      CursorIndex = FMath::Clamp(--CursorIndex, 0, InputText.Len());
      break;
    case EKeyType::RIGHT:
      CursorIndex = FMath::Clamp(++CursorIndex, 0, InputText.Len());
      break;
    case EKeyType::END:
      CursorIndex = InputText.Len();
      break;
  }

  OnInputUpdatedDelegate.Broadcast(InputText, CursorIndex);
}

void UKeyboardWidget::SetShifted() {
  for (auto& key : Keys) {
    if (key->Data.IsShiftable()) {
      key->SecondaryLabel->SetText(FText::FromString(key->Data.PrimaryLabel));
      key->PrimaryLabel->SetText(FText::FromString(key->Data.SecondaryLabel));
    }
    if (key->Data.Type == EKeyType::SHIFT) {
      key->ShiftIcon->SetVisibility(ESlateVisibility::Hidden);
      key->ShiftLockIcon->SetVisibility(ESlateVisibility::Visible);
    }
  }
}

void UKeyboardWidget::SetUnshifted() {
  for (auto& key : Keys) {
    if (key->Data.IsShiftable()) {
      key->PrimaryLabel->SetText(FText::FromString(key->Data.PrimaryLabel));
      key->SecondaryLabel->SetText(FText::FromString(key->Data.SecondaryLabel));
    }
    if (key->Data.Type == EKeyType::SHIFT) {
      key->ShiftLockIcon->SetVisibility(ESlateVisibility::Hidden);
      key->ShiftIcon->SetVisibility(ESlateVisibility::Visible);
    }
  }
}


void UKeyboardWidget::MakeKeys() {
  // what row, what column, how many columns to skip
  TMap<TPair<int8, int8>, int8> skipCols;

  int8 row = 0, col = 0;
  for (auto& key : KeyMap) {
    // skip columns if a previous key spanned more than one row
    if (skipCols.Contains(TPair<int8, int8>(row, col))) {
      col += skipCols[TPair<int8, int8>(row, col)];
      if (col >= MaxCols) {
        col = 0;
        row++;
      }
    }
    col += key.ColOffset;

    MakeKey(key, row, col);

    // set up column skipping for rows that this key spans over
    if (key.RowSpan > 1) {
      for (int8 i = 1; i < key.RowSpan; i++) {
        skipCols.Add(TPair<int8, int8>(row + i, col), key.ColSpan);
      }
    }

    col += key.ColSpan;
    if (col >= MaxCols) {
      col = 0;
      row++;
    }
  }
}

void UKeyboardWidget::MakeKey(KeyInfo& Key, int8 Row, int8 Col) {
  UKeyboardKey* KeyWidget = CreateWidget<UKeyboardKey>(this, KeyClass);
  Keys.Add(KeyWidget);

  KeyWidget->SizeBox->SetHeightOverride(Key.RowSpan * 76);
  KeyWidget->SizeBox->SetWidthOverride(Key.ColSpan * 76);
  KeyWidget->Data = Key;
  
  if (Key.Type == EKeyType::RAW) {
    KeyWidget->PrimaryLabel->SetText(FText::FromString(Key.PrimaryLabel));
    KeyWidget->SecondaryLabel->SetText(FText::FromString(Key.SecondaryLabel));
  } else {
    KeyWidget->PrimaryLabel->SetVisibility(ESlateVisibility::Hidden);
    KeyWidget->SecondaryLabel->SetVisibility(ESlateVisibility::Hidden);
  }

  switch (Key.Type) {
    case EKeyType::CONFIRM:
      KeyWidget->ConfirmIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::BACKSPACE:
      KeyWidget->BackspaceIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::SHIFT:
      KeyWidget->ShiftIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::SPACE:
      KeyWidget->SpaceIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::HOME:
      KeyWidget->HomeIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::LEFT:
      KeyWidget->LeftIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::RIGHT:
      KeyWidget->RightIcon->SetVisibility(ESlateVisibility::Visible);
      break;
    case EKeyType::END:
      KeyWidget->EndIcon->SetVisibility(ESlateVisibility::Visible);
      break;
  }

  KeyWidget->OnClickedDelegate.AddUniqueDynamic(this, &UKeyboardWidget::HandleKeyReleased);
  
  UGridSlot* slot = Cast<UGridSlot>(GridPanel->AddChild(KeyWidget));
  slot->SetRow(Row);
  slot->SetColumn(Col);
  slot->SetRowSpan(Key.RowSpan);
  slot->SetColumnSpan(Key.ColSpan);
}