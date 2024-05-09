#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyboardWidget.generated.h"

class UGridPanel;
class UKeyboardKey;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputConfirmedSignature, FString, Input);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInputUpdatedSignature, FString, Input, int8, CursorIndex);

UENUM()
enum class EKeyType: int8 {
  RAW,
  BACKSPACE,
  CONFIRM,
  SHIFT,
  HOME,
  LEFT,
  RIGHT,
  END,
  SPACE
};

struct KeyInfo {
  EKeyType Type;
  FString PrimaryLabel{""};
  FString SecondaryLabel{""};
  int8 Row{0};
  int8 Col{0};
  int8 ColSpan{1};
  int8 RowSpan{1};
  int8 ColOffset{0};
  
  bool IsShiftable() { return !SecondaryLabel.IsEmpty(); }
  
  KeyInfo() {}
  KeyInfo(FString _PrimaryLabel, FString _SecondaryLabel)
    : PrimaryLabel(_PrimaryLabel), SecondaryLabel(_SecondaryLabel) { Type = EKeyType::RAW; }
  KeyInfo(EKeyType _Type, FString _PrimaryLabel, FString _SecondaryLabel, int8 _ColSpan, int8 _ColOffset)
    : Type(_Type), PrimaryLabel(_PrimaryLabel), SecondaryLabel(_SecondaryLabel), ColSpan(_ColSpan), ColOffset(_ColOffset) {}
  KeyInfo(EKeyType _Type, FString _PrimaryLabel, FString _SecondaryLabel, int8 _ColSpan, int8 _RowSpan, int8 _ColOffset)
    : Type(_Type), PrimaryLabel(_PrimaryLabel), SecondaryLabel(_SecondaryLabel), ColSpan(_ColSpan), RowSpan(_RowSpan), ColOffset(_ColOffset) {}
};


UCLASS()
class OSC3_API UKeyboardWidget : public UUserWidget {
	GENERATED_BODY()
	
public:
  TSubclassOf<class UUserWidget> KeyClass;
  void Init();
  void SetShifted();
  void SetUnshifted();
  
  FInputUpdatedSignature OnInputUpdatedDelegate;
  FInputConfirmedSignature OnInputConfirmedDelegate;
  
  void ClearInput() {
    InputText = "";
    CursorIndex = 0;
    OnInputUpdatedDelegate.Broadcast(InputText, CursorIndex);
  }

  void SetInput(FString Input) {
    InputText = Input;
    CursorIndex = Input.Len();
    OnInputUpdatedDelegate.Broadcast(InputText, CursorIndex);
  }

protected:
	virtual void NativeConstruct() override;	
  
  UPROPERTY(meta = (BindWidget))
  UGridPanel* GridPanel;

private:
  UFUNCTION()
  void HandleKeyReleased(UKeyboardKey* Key);
  
  int8 CursorIndex{0};
  FString InputText{""};
  
  bool bShifted{false};

  int8 MaxCols{12};
  void MakeKeys();
  void MakeKey(KeyInfo& Key, int8 Row, int8 Col);
  TArray<UKeyboardKey*> Keys;
  TArray<KeyInfo> KeyMap{
    // row 1
    {"1", "!"},
    {"2", "@"},
    {"3", "#"},
    {"4", "$"},
    {"5", "%"},
    {"6", "&"},
    {"7", "_"},
    {"8", "-"},
    {"9", "+"},
    {"0", "="},
    { EKeyType::BACKSPACE, "BKSP", "", 2, 0},
    
    // row two
    {"q", "Q"},
    {"w", "W"},
    {"e", "E"},
    {"r", "R"},
    {"t", "T"},
    {"y", "Y"},
    {"u", "U"},
    {"i", "I"},
    {"o", "O"},
    {"p", "P"},
    { EKeyType::CONFIRM, "RTRN", "", 2, 2, 0},
    
    // row three
    {"a", "A"},
    {"s", "S"},
    {"d", "D"},
    {"f", "F"},
    {"g", "G"},
    {"h", "H"},
    {"j", "J"},
    {"k", "K"},
    {"l", "L"},
    {"'", ""},

    // row 4
    { EKeyType::RAW, "z", "Z", 1, 1},
    {"x", "X"},
    {"c", "C"},
    {"v", "V"},
    {"b", "B"},
    {"n", "N"},
    {"m", "M"},
    {".", ""},
    {",", ""},
    { EKeyType::SHIFT, "SHFT", "", 2, 0},

    // row 5
    {"(", ""},
    {")", ""},
    {"[", ""},
    {"]", ""},
    { EKeyType::SPACE, "SPACE", "", 4, 0},
    { EKeyType::HOME, "|<-", "", 1, 0},
    { EKeyType::LEFT, "<-", "", 1, 0},
    { EKeyType::RIGHT, "->", "", 1, 0},
    { EKeyType::END, "->|", "", 1, 0},
  };
};
