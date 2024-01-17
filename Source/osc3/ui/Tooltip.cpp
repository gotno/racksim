#include "ui/Tooltip.h"

#include "Components/TextBlock.h"

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