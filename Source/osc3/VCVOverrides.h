#pragma once
#include "Internationalization/Regex.h"

struct VCVOverrides {
  bool getBodyColor(FString Brand, FLinearColor& BodyColor) {
    if (BodyColors.Contains(Brand)) {
      BodyColor = BodyColors[Brand];
      return true;
    }
    return false;
  }

  FLinearColor getMatchingColor(FString brand, FString svgPath = TEXT("")) {
    FString filename = getFilename(svgPath);

    if (filename.IsEmpty()) {
      if (brandColors.Contains(brand)) return brandColors[brand];
      return brandColors["default"];
    }

    if (brandSvgColors.Contains(brand) && brandSvgColors[brand].Contains(filename)) {
      return brandSvgColors[brand][filename];
    }
    
    if (svgColors.Contains(filename)) return svgColors[filename];
    return svgColors["default"];
  }

  FVector getScaleMultiplier(FString brand, FString svgPath) {
    FString filename = getFilename(svgPath);

    if (scaleMultipliers.Contains(brand) && scaleMultipliers[brand].Contains(filename)) {
      return scaleMultipliers[brand][filename];
    }
    return FVector::OneVector;
  }

  FVector2D getUVOverride(FString brand) {
    if (UVOverrides.Contains(brand)) return UVOverrides[brand];
    return FVector2D(1, 1);
  }
  
  VCVOverrides() {
    BodyColors.Add("alef's bits", FLinearColor(1.f, 1.f, 1.f));

    brandColors.Add("default", FLinearColor(0.902f, 0.902f, 0.902f));
    brandColors.Add("Befaco", FLinearColor(0.09f, 0.09f, 0.09f));
    brandColors.Add("Instruō", FLinearColor(0.118f, 0.118f, 0.118f));
    brandColors.Add("NANO Modules", FLinearColor(0.004f, 0.004f, 0.f));
    brandColors.Add("Valley", FLinearColor(0.157f, 0.157f, 0.157f));
    brandColors.Add("Fehler Fabrik", FLinearColor(0.165f, 0.176f, 0.188f));
    brandColors.Add("NANO Modules", FLinearColor::Black);
    
    brandSvgColors.Add("Befaco", ColorMap());
    brandSvgColors["Befaco"].Add("BananutRed", FLinearColor(0.863f, 0.078f, 0.078f));
    brandSvgColors["Befaco"].Add("BananutBlack", FLinearColor(0.18f, 0.18f, 0.18f));

    brandSvgColors.Add("Instruō", ColorMap());
    brandSvgColors["Instruō"].Add("brassnut", FLinearColor(0.867f, 0.761f, 0.557f));
    brandSvgColors["Instruō"].Add("brasstrimpot", FLinearColor(0.867f, 0.761f, 0.557f));
    
    svgColors.Add("default", FLinearColor::Black);
    svgColors.Add("PJ301M", FLinearColor(0.878f, 0.878f, 0.878f));
    
    scaleMultipliers.Add("Instruō", VectorMap());
    scaleMultipliers["Instruō"].Add("brasstrimpot", FVector(0.05f, 1, 1));

    UVOverrides.Add("NANO Modules", FVector2D(0.989f, 1.f));
  };

private:
  TMap<FString, FVector2D> UVOverrides;

  typedef TMap<FString, FVector> VectorMap;
  TMap<FString, VectorMap> scaleMultipliers;

  typedef TMap<FString, FLinearColor> ColorMap;
  ColorMap brandColors;
  ColorMap BodyColors;
  ColorMap svgColors;
  TMap<FString, ColorMap> brandSvgColors;
  
  FString getFilename(FString& filepath) {
    // ugh, https://stackoverflow.com/questions/58872769/regex-to-get-filename-with-or-without-extension-from-a-path
    FRegexPattern filenameRegex(TEXT("^(?:[^\\/\r\n]*\\/)*([^\\/\r\n]+?|)(?=(?:\\.[^\\/\r\n.]*)?$)"));
    FString filename;

    FRegexMatcher filenameMatcher(filenameRegex, filepath);
    if (filenameMatcher.FindNext()) filename = filenameMatcher.GetCaptureGroup(1);
    return filename;
  }
};