// Sample LCARS Report Generator - For Demo/Screenshot Purposes
#include "LCARSHTMLGenerator.h"
#include "Misc/Paths.h"
#include "Nexus/Palantir/Public/PalantirTypes.h"

namespace LCARSSample
{
	FReportData GenerateSampleData()
	{
		FReportData Data;
		
		// Basic Info
		Data.Title = TEXT("Nexus Demo Suite - API & Integration Tests");
		Data.Timestamp = FDateTime::Now();
		Data.TotalTests = 15;
		Data.PassedTests = 12;
		Data.FailedTests = 3;
		Data.TotalDuration = 8.745f;
		
		// API Metrics
		Data.APIMetrics.TotalRequests = 47;
		Data.APIMetrics.SuccessfulRequests = 43;
		Data.APIMetrics.FailedRequests = 4;
		Data.APIMetrics.AvgResponseTimeMs = 127.3f;
		
		// Status Codes
		Data.APIMetrics.StatusCodeDistribution.Add(200, 28);
		Data.APIMetrics.StatusCodeDistribution.Add(201, 10);
		Data.APIMetrics.StatusCodeDistribution.Add(204, 5);
		Data.APIMetrics.StatusCodeDistribution.Add(400, 2);
		Data.APIMetrics.StatusCodeDistribution.Add(500, 2);
		
		// Tested Endpoints
		Data.APIMetrics.TestedEndpoints.Add(TEXT("https://api.example.com/v1/users"));
		Data.APIMetrics.TestedEndpoints.Add(TEXT("https://api.example.com/v1/posts"));
		Data.APIMetrics.TestedEndpoints.Add(TEXT("https://graphql.example.com/query"));
		Data.APIMetrics.TestedEndpoints.Add(TEXT("https://playfab.com/CloudScript/ExecuteFunction"));
		
		// Endpoint Response Times
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("GET /v1/users"), 89.2f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("POST /v1/users"), 145.7f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("GET /v1/posts?userId=1"), 67.3f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("POST /v1/posts"), 203.1f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("GraphQL: GetPlayer"), 112.5f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("GraphQL: UpdateInventory"), 167.8f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("PlayFab: AwardAchievement"), 234.6f);
		Data.APIMetrics.EndpointResponseTimes.Add(TEXT("PlayFab: GetLeaderboard"), 89.1f);
		
		// Performance Metrics
		Data.PerfMetrics.AvgFPS = 58.3f;
		Data.PerfMetrics.MinFPS = 45.2f;
		Data.PerfMetrics.MaxFPS = 60.0f;
		Data.PerfMetrics.PeakMemoryMB = 1847.5f;
		Data.PerfMetrics.HitchCount = 3;
		
		// Test Results
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_GetRequest_Success");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.245f;
			Test.TraceID = TEXT("TR-001-4A7B9C2D");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_PostWithJSON_Success");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.312f;
			Test.TraceID = TEXT("TR-002-8E3F1A5B");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_GraphQL_QueryUsers");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.189f;
			Test.TraceID = TEXT("TR-003-2C9D4E1F");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_GraphQL_WithVariables");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.201f;
			Test.TraceID = TEXT("TR-004-7B2A8C3D");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_ExpectStatus404_Success");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.134f;
			Test.TraceID = TEXT("TR-005-9F1E5B4A");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_JSONPath_Validation");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.278f;
			Test.TraceID = TEXT("TR-006-3D8C2A1F");
			Test.Artifacts.Add(TEXT("Traces/TR-006-3D8C2A1F.json"));
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_Retry_ExponentialBackoff");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 1.523f;
			Test.TraceID = TEXT("TR-007-5A9B1C7E");
			Test.Artifacts.Add(TEXT("Traces/TR-007-5A9B1C7E.json"));
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_AsyncRequest_Success");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.456f;
			Test.TraceID = TEXT("TR-008-8C3D2F1A");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PlayFab_CloudScript_ExecuteFunction");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.389f;
			Test.TraceID = TEXT("TR-009-1F7B9A2D");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PlayFab_GetLeaderboard_Pagination");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.512f;
			Test.TraceID = TEXT("TR-010-4E8A3C1B");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("GameAnalytics_TrackEvent_Success");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.267f;
			Test.TraceID = TEXT("TR-011-9C2D5F1A");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("Sentry_ErrorCapture_Integration");
			Test.Status = TEXT("PASSED");
			Test.DurationSeconds = 0.198f;
			Test.TraceID = TEXT("TR-012-7A1B8E3C");
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_InvalidEndpoint_404");
			Test.Status = TEXT("FAILED");
			Test.DurationSeconds = 2.145f;
			Test.TraceID = TEXT("TR-013-2F9C4A1D");
			Test.ErrorMessage = TEXT("Expected status 200 but got 404. Response body indicates endpoint not found.");
			Test.Artifacts.Add(TEXT("Traces/TR-013-2F9C4A1D.json"));
			Test.Artifacts.Add(TEXT("Screenshots/error-404-response.png"));
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PlayFab_InvalidToken_Authentication");
			Test.Status = TEXT("FAILED");
			Test.DurationSeconds = 0.523f;
			Test.TraceID = TEXT("TR-014-8B3E1A9C");
			Test.ErrorMessage = TEXT("Authentication failed: Invalid or expired session token (PlayFab error 1074).");
			Test.Artifacts.Add(TEXT("Traces/TR-014-8B3E1A9C.json"));
			Data.Tests.Add(Test);
		}
		
		{
			FTestResult Test;
			Test.Name = TEXT("PalantirRequest_Timeout_SlowEndpoint");
			Test.Status = TEXT("FAILED");
			Test.DurationSeconds = 5.123f;
			Test.TraceID = TEXT("TR-015-5C1D7F2A");
			Test.ErrorMessage = TEXT("Request timeout after 5000ms. Endpoint did not respond within configured timeout.");
			Test.Artifacts.Add(TEXT("Traces/TR-015-5C1D7F2A.json"));
			Test.Artifacts.Add(TEXT("Logs/timeout-analysis.txt"));
			Data.Tests.Add(Test);
		}
		
		return Data;
	}
	
	void GenerateSampleReport()
	{
		FReportData Data = GenerateSampleData();
		FString OutputPath = FPaths::ProjectDir() / TEXT("TestReports") / TEXT("LCARS_Demo_Report.html");
		
		if (FLCARSHTMLGenerator::SaveToFile(Data, OutputPath))
		{
			UE_LOG(LogTemp, Display, TEXT("✅ Sample LCARS report generated: %s"), *OutputPath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("❌ Failed to generate sample report at: %s"), *OutputPath);
		}
	}
}

// Commandlet for generating sample report
UCLASS()
class UGenerateLCARSReportCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override
	{
		LCARSSample::GenerateSampleReport();
		return 0;
	}
};
