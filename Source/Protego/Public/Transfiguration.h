#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Transfiguration.generated.h"

// Result of a single accessibility check
USTRUCT(BlueprintType)
struct FAccessibilityCheckResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString CheckName;

    UPROPERTY(BlueprintReadWrite)
    bool bPassed = false;

    UPROPERTY(BlueprintReadWrite)
    FString Message;

    UPROPERTY(BlueprintReadWrite)
    float Score = 0.0f; // 0-1
};

/**
 * Transfiguration — Accessibility Test Suite (color-blind, subtitles, input remap, contrast)
 */
UCLASS()
class UTransfiguration : public UObject
{
    GENERATED_BODY()

public:
    // Run all accessibility checks
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static void RunAccessibilityAudit();

    // Check color-blind mode simulation (Deuteranopia, Protanopia, Tritanopia)
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static bool CheckColorBlindModes();

    // Verify subtitle/caption presence and timing
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static bool CheckSubtitlePresence();

    // Validate input remapping and accessible controls
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static bool CheckInputRemapping();

    // Check UI contrast ratios (WCAG 2.1 AA standard: 4.5:1 for text)
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static bool CheckContrastRatios();

    // Export accessibility report to JSON + HTML
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static void ExportAccessibilityArtifact(const FString& OutputPath = TEXT(""));

    // Get overall accessibility score (0-1)
    UFUNCTION(BlueprintCallable, Category = "Accessibility|Transfiguration")
    static float GetAccessibilityScore();
};
