#include "Tooltip.h"

#include "Components/TextBlock.h"

void UTooltip::SetLineOne(FString NewLineOne) {
  LineOne->SetText(FText::FromString(NewLineOne));
  LineOne->SetVisibility(ESlateVisibility::Collapsed);
}

void UTooltip::SetLineTwo(FString NewLineTwo) {
  LineTwo->SetText(FText::FromString(NewLineTwo));
}

void UTooltip::SetText(FString InLineOne, FString InLineTwo, bool bEmphasis, bool bSubtitle) {
  if (InLineTwo.IsEmpty()) {
    LineTwo->SetVisibility(ESlateVisibility::Collapsed);
  } else {
    LineTwo->SetVisibility(ESlateVisibility::Visible);

    FSlateFontInfo fontInfo = LineTwo->GetFont();
    if (bEmphasis) {
      fontInfo.Size = EmphasisTextSize;
    } else if (bSubtitle) {
      fontInfo.Size = SubtitleTextSize;
    } else {
      fontInfo.Size = DefaultTextSize;
    }

    LineTwo->SetFont(fontInfo);
    LineTwo->SetText(FText::FromString(InLineTwo));
  }

  LineOne->SetText(FText::FromString(InLineOne));
}