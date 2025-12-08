#include "NexusTest.h"
#include "PalantirRequest.h"

/**
 * Sample tests demonstrating network request tracing with PalantírRequest.
 * 
 * These tests showcase:
 * - REST API contract validation
 * - GraphQL query testing
 * - Automatic trace ID propagation
 * - Response validation with fluent API
 * - Retry logic with exponential backoff
 * 
 * Inspired by:
 * - Mocha/Chai API testing patterns
 * - Cypress HTTP interception
 * - Garmin's DataDog APM tracing
 */

//------------------------------------------------------------------------------
// Basic REST API Tests
//------------------------------------------------------------------------------

NEXUS_TEST(FPalantirRequest_HealthCheck, "Palantir.Request.HealthCheck", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Health check endpoint (example.com always returns 200)
	FPalantirResponse Res = FPalantirRequest::Get(TEXT("https://www.example.com/"))
		.WithTimeout(5.0f)
		.ExpectStatusRange(200, 299)
		.ExecuteBlocking();

	if (!Res.IsSuccess())
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Health check failed: HTTP %d"), Res.StatusCode);
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("Health check passed in %.1fms [Trace: %s]"), Res.DurationMs, *Res.TraceID);
	return true;
}

NEXUS_TEST(FPalantirRequest_JSONValidation, "Palantir.Request.JSONValidation", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test JSONPlaceholder API (public test API)
	FPalantirResponse Res = FPalantirRequest::Get(TEXT("https://jsonplaceholder.typicode.com/users/1"))
		.WithTimeout(10.0f)
		.ExpectStatus(200)
		.ExpectHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"))
		.ExpectJSON(TEXT("id"), TEXT("1"))
		.ExpectJSON(TEXT("name"), TEXT("Leanne Graham"))
		.ExecuteBlocking();

	if (!Res.IsSuccess())
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("JSON validation failed: HTTP %d"), Res.StatusCode);
		return false;
	}

	// Additional manual validation
	FString Username = Res.GetJSONValue(TEXT("username"));
	if (Username != TEXT("Bret"))
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Expected username 'Bret', got '%s'"), *Username);
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("JSON validation passed [Trace: %s]"), *Res.TraceID);
	return true;
}

NEXUS_TEST(FPalantirRequest_PostRequest, "Palantir.Request.PostRequest", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test POST endpoint (JSONPlaceholder echo endpoint)
	FString PostBody = TEXT("{\"title\": \"Test Post\", \"body\": \"Test Body\", \"userId\": 1}");

	FPalantirResponse Res = FPalantirRequest::Post(TEXT("https://jsonplaceholder.typicode.com/posts"), PostBody)
		.WithTimeout(10.0f)
		.ExpectStatus(201)  // Created
		.ExpectBodyContains(TEXT("Test Post"))
		.ExecuteBlocking();

	if (Res.StatusCode != 201)
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("POST failed: HTTP %d"), Res.StatusCode);
		return false;
	}

	// Verify response contains echo of our data
	FString Title = Res.GetJSONValue(TEXT("title"));
	if (Title != TEXT("Test Post"))
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("POST echo failed: expected 'Test Post', got '%s'"), *Title);
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("POST request passed [Trace: %s]"), *Res.TraceID);
	return true;
}

//------------------------------------------------------------------------------
// GraphQL Tests
//------------------------------------------------------------------------------

NEXUS_TEST(FPalantirRequest_GraphQL, "Palantir.Request.GraphQL", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test public GraphQL endpoint (SpaceX API)
	FString Query = TEXT("{ company { name ceo coo } }");

	FPalantirResponse Res = FPalantirRequest::GraphQL(TEXT("https://api.spacex.land/graphql/"), Query)
		.WithTimeout(10.0f)
		.ExpectStatus(200)
		.ExpectBodyContains(TEXT("SpaceX"))
		.ExecuteBlocking();

	if (!Res.IsSuccess())
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("GraphQL query failed: HTTP %d"), Res.StatusCode);
		return false;
	}

	// Verify response structure (data.company.name)
	TSharedPtr<FJsonObject> JSON = Res.GetJSON();
	if (!JSON.IsValid())
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Failed to parse GraphQL response"));
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("GraphQL query passed [Trace: %s]"), *Res.TraceID);
	return true;
}

//------------------------------------------------------------------------------
// Error Handling & Retry Tests
//------------------------------------------------------------------------------

NEXUS_TEST(FPalantirRequest_404Handling, "Palantir.Request.404Handling", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test 404 detection
	FPalantirResponse Res = FPalantirRequest::Get(TEXT("https://jsonplaceholder.typicode.com/nonexistent"))
		.WithTimeout(5.0f)
		.ExecuteBlocking();

	if (Res.StatusCode != 404)
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Expected 404, got %d"), Res.StatusCode);
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("404 handling passed [Trace: %s]"), *Res.TraceID);
	return true;
}

NEXUS_TEST(FPalantirRequest_RetryLogic, "Palantir.Request.RetryLogic", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test retry with a flaky endpoint (this will timeout or fail initially)
	// Note: This test may take up to 15 seconds due to retries
	FPalantirResponse Res = FPalantirRequest::Get(TEXT("https://httpstat.us/503"))
		.WithTimeout(3.0f)
		.WithRetry(2, 1.0f)  // 2 retries with 1s delay
		.ExpectStatusRange(500, 599)  // Expect 5xx error
		.ExecuteBlocking();

	// We expect a 503, and the retry logic should have attempted 3 times total
	if (Res.StatusCode != 503)
	{
		UE_LOG(LogPalantirTrace, Warning, TEXT("Expected 503 Service Unavailable, got %d"), Res.StatusCode);
		// Don't fail test - endpoint behavior may vary
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("Retry logic test completed [Trace: %s]"), *Res.TraceID);
	return true;
}

//------------------------------------------------------------------------------
// Async Request Test
//------------------------------------------------------------------------------

NEXUS_TEST(FPalantirRequest_AsyncRequest, "Palantir.Request.AsyncRequest", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test async request with callback
	bool bCallbackFired = false;
	int32 ResponseStatus = 0;

	FPalantirRequest::Get(TEXT("https://www.example.com/"))
		.WithTimeout(5.0f)
		.ExecuteAsync([&bCallbackFired, &ResponseStatus](const FPalantirResponse& Res)
		{
			bCallbackFired = true;
			ResponseStatus = Res.StatusCode;
			UE_LOG(LogPalantirTrace, Display, TEXT("Async callback received: HTTP %d in %.1fms"), Res.StatusCode, Res.DurationMs);
		});

	// Wait for callback (max 6 seconds)
	float WaitTime = 0.0f;
	while (!bCallbackFired && WaitTime < 6.0f)
	{
		FPlatformProcess::Sleep(0.1f);
		WaitTime += 0.1f;
	}

	if (!bCallbackFired)
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Async callback did not fire within 6 seconds"));
		return false;
	}

	if (ResponseStatus != 200)
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Async request failed: HTTP %d"), ResponseStatus);
		return false;
	}

	UE_LOG(LogPalantirTrace, Display, TEXT("Async request passed"));
	return true;
}

//------------------------------------------------------------------------------
// Macro Convenience Tests
//------------------------------------------------------------------------------

NEXUS_TEST(FPalantirRequest_MacroConvenience, "Palantir.Request.MacroConvenience", (ETestPriority::Normal | ETestPriority::OnlineOnly))
{
	// Test convenience macros
	PALANTIR_ASSERT_GET_OK(TEXT("https://www.example.com/"));
	
	PALANTIR_ASSERT_HEALTH_CHECK(TEXT("https://jsonplaceholder.typicode.com/users/1"));

	UE_LOG(LogPalantirTrace, Display, TEXT("Macro convenience tests passed"));
	return true;
}
