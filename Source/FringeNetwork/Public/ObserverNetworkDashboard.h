#pragma once
#include "CoreMinimal.h"
#include "ObserverNetworkDashboard.generated.h"

/**
 * Dashboard rendering backend
 * Controls how the live dashboard is displayed during test execution
 */
UENUM(BlueprintType)
enum class EDashboardBackend : uint8
{
    ImGui        UMETA(DisplayName = "ImGui - Best performance, requires ImGui plugin"),
    Slate        UMETA(DisplayName = "Slate - Native UE UI, always available"),
    HTMLOnly     UMETA(DisplayName = "HTML Only - Fallback, no live display"),
    Auto         UMETA(DisplayName = "Auto - ImGui if available, fallback to HTML")
};

UCLASS()
class FRINGENETWORK_API UObserverNetworkDashboard : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Initialize dashboard with specified rendering backend
     * @param Backend - Which rendering system to use (ImGui, Slate, HTML, or Auto)
     */
    static void Initialize(EDashboardBackend Backend = EDashboardBackend::Auto);

    /**
     * Call every frame when active to update live dashboard
     * No-op if backend doesn't support live rendering (HTMLOnly)
     */
    static void UpdateLiveDashboard();

    /**
     * Log a safety event (always recorded, may be displayed live if backend supports it)
     */
    static void LogSafetyEvent(const FString& EventType, const FString& Details);

    /**
     * Generate final web report (always generated regardless of backend)
     */
    static void GenerateWebReport();

    /**
     * Get currently active rendering backend
     */
    static EDashboardBackend GetActiveBackend();

private:
    /**
     * Load HTML template from various sources
     * Tries: source directory, alternate source path, content directory, then embedded fallback
     */
    static FString LoadHTMLTemplate();

    /**
     * Get embedded HTML template (works in all build configurations)
     * Used as fallback when template file cannot be found
     */
    static FString GetEmbeddedHTMLTemplate();
};
