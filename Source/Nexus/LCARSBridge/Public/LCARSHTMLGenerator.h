#pragma once

#include "CoreMinimal.h"

/**
 * Enhanced LCARS HTML Report Generator with API Metrics Visualization
 * 
 * Generates Starfleet-themed HTML dashboards with:
 * - Test pass/fail summary
 * - Performance metrics graphs
 * - API request/response timeline
 * - Status code distribution charts
 * - Network metrics visualization
 */
class NEXUS_API FLCARSHTMLGenerator
{
public:
	struct FAPIMetrics
	{
		int32 TotalRequests = 0;
		int32 SuccessfulRequests = 0;
		int32 FailedRequests = 0;
		float AvgResponseTimeMs = 0.0f;
		TMap<int32, int32> StatusCodeDistribution;  // Status code -> count
		TArray<FString> TestedEndpoints;
		TMap<FString, float> EndpointResponseTimes;  // Endpoint -> avg time
	};

	struct FPerformanceMetrics
	{
		float AvgFPS = 0.0f;
		float MinFPS = 0.0f;
		float MaxFPS = 0.0f;
		float PeakMemoryMB = 0.0f;
		int32 HitchCount = 0;
	};

	struct FTestResult
	{
		FString Name;
		FString Status;  // PASSED, FAILED, SKIPPED
		float DurationSeconds = 0.0f;
		FString ErrorMessage;
		TArray<FString> Artifacts;
		FString TraceID;
	};

	struct FReportData
	{
		FString Title = TEXT("LCARS Test Report");
		FDateTime Timestamp;
		TArray<FTestResult> Tests;
		FAPIMetrics APIMetrics;
		FPerformanceMetrics PerfMetrics;
		int32 TotalTests = 0;
		int32 PassedTests = 0;
		int32 FailedTests = 0;
		int32 SkippedTests = 0;
		float TotalDuration = 0.0f;
	};

	/**
	 * Generate HTML report with embedded charts and visualizations
	 */
	static FString GenerateHTML(const FReportData& Data);

	/**
	 * Save HTML report to file
	 */
	static bool SaveToFile(const FReportData& Data, const FString& OutputPath);

private:
	static FString GenerateCSS();
	static FString GenerateJavaScript();
	static FString GenerateTestSummarySection(const FReportData& Data);
	static FString GenerateTestDetailsSection(const FReportData& Data);
	static FString GenerateAPIMetricsSection(const FReportData& Data);
	static FString GeneratePerformanceMetricsSection(const FReportData& Data);
	static FString GenerateAPITimelineChart(const FReportData& Data);
	static FString GenerateStatusCodePieChart(const FReportData& Data);
};
