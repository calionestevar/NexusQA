#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Containers/List.h"

/**
 * FPalantirTrace maintains a unique trace ID (correlation ID) for each test execution.
 * This ID is injected into logs, HTTP headers, and metrics to enable cross-system tracing
 * (e.g., game logs → DataDog → backend logs → database queries).
 *
 * Named after the Palantíri — the seeing-stones of Middle-earth that allow communication
 * across vast distances. Perfect metaphor for distributed tracing.
 *
 * Pattern inspired by:
 * - Cerner's custom test identifier system (log correlation)
 * - Garmin's DataDog User-Agent tracing
 */
class NEXUS_API FPalantirTrace
{
public:
	/**
	 * Generate a new unique trace ID for a test execution.
	 * Format: "nexus-test-<UUID>"
	 * Example: "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e"
	 */
	static FString GenerateTraceID();

	/**
	 * Set the trace ID for the current test context.
	 * This ID will be automatically injected into:
	 * - Unreal logs (via UE_LOG with trace tag)
	 * - HTTP request headers (X-Trace-ID, User-Agent)
	 * - ArgusLens performance metrics
	 * - Network event logs
	 */
	static void SetCurrentTraceID(const FString& TraceID);

	/**
	 * Retrieve the current test's trace ID.
	 * Returns empty string if no test is active.
	 */
	static FString GetCurrentTraceID();

	/**
	 * Clear the trace context (typically called after test completion).
	 */
	static void Clear();

	/**
	 * Add a breadcrumb event to the trace (e.g., "started asset loading", "network error at 5.2s").
	 * Useful for timeline reconstruction during debugging.
	 */
	static void AddBreadcrumb(const FString& EventName, const FString& Details = TEXT(""));

	/**
	 * Get all breadcrumbs for the current trace.
	 */
	static TArray<TPair<double, FString>> GetBreadcrumbs();

	/**
	 * Export trace metadata to JSON (for DataDog, ELK, or other APM systems).
	 */
	static FString ExportToJSON();

private:
	// Thread-local trace ID storage
	static thread_local FString CurrentTraceID;
	
	// Breadcrumb events: (timestamp, message)
	static thread_local TArray<TPair<double, FString>> CurrentBreadcrumbs;
	static thread_local double TraceStartTime;
};

/**
 * RAII guard for trace context. Automatically generates a trace ID on construction
 * and clears it on destruction.
 * 
 * Usage:
 *   {
 *       FPalantirTraceGuard Guard;  // Generates trace ID
 *       // ... test code ...
 *   }  // Trace context cleaned up automatically
 */
class NEXUS_API FPalantirTraceGuard
{
public:
	FPalantirTraceGuard();
	~FPalantirTraceGuard();

	const FString& GetTraceID() const { return TraceID; }

private:
	FString TraceID;
};

/**
 * Macro to inject current trace ID into log messages.
 * Usage: UE_LOG_TRACE(LogNexus, Log, TEXT("Something happened"));
 */
#define UE_LOG_TRACE(Category, Verbosity, Format, ...) \
	{ \
		FString TracePrefix = FPalantirTrace::GetCurrentTraceID().IsEmpty() \
			? FString(TEXT("")) \
			: FString::Printf(TEXT("[%s] "), *FPalantirTrace::GetCurrentTraceID()); \
		UE_LOG(Category, Verbosity, TEXT("%s") Format, *TracePrefix, ##__VA_ARGS__); \
	}

/**
 * Macro to log breadcrumb events (timeline markers).
 * Usage: PALANTIR_BREADCRUMB(TEXT("LoadedAsset"), FString::Printf(TEXT("Asset: %s, Time: %.2fs"), *AssetName, ElapsedTime));
 */
#define PALANTIR_BREADCRUMB(EventName, Details) \
	FPalantirTrace::AddBreadcrumb(EventName, Details)
