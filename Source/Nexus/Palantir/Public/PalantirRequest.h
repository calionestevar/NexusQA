#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "PalantirTrace.h"

/**
 * Network request wrapper with automatic trace ID injection and response validation.
 * 
 * Inspired by:
 * - Garmin's DataDog APM tracing via User-Agent headers
 * - Mocha/Chai API contract testing patterns
 * - Cypress HTTP interception and assertion patterns
 * 
 * Features:
 * - Automatic X-Trace-ID header injection
 * - Request/response logging with breadcrumbs
 * - Fluent assertion API for status codes, headers, JSON bodies
 * - GraphQL support with query validation
 * - Retry logic with exponential backoff
 * 
 * Example:
 *   FPalantirRequest::Get("https://api.example.com/users/123")
 *       .WithHeader("Authorization", "Bearer <token>")
 *       .ExpectStatus(200)
 *       .ExpectJSON("$.name", "John Doe")
 *       .ExecuteBlocking();
 */

/**
 * HTTP response wrapper with validation helpers.
 */
struct NEXUS_API FPalantirResponse
{
	int32 StatusCode;
	FString Body;
	TMap<FString, FString> Headers;
	float DurationMs;
	FString TraceID;

	/** Check if response is successful (2xx status code) */
	bool IsSuccess() const { return StatusCode >= 200 && StatusCode < 300; }

	/** Parse body as JSON object */
	TSharedPtr<FJsonObject> GetJSON() const;

	/** Get JSON value at JSONPath (simplified: dot notation like "user.name") */
	FString GetJSONValue(const FString& JSONPath) const;

	/** Validate response against expectations */
	bool Validate(FString& OutError) const;
};

/**
 * HTTP request builder with fluent API and automatic tracing.
 */
class NEXUS_API FPalantirRequest
{
public:
	/** Create GET request */
	static FPalantirRequest Get(const FString& URL);

	/** Create POST request */
	static FPalantirRequest Post(const FString& URL, const FString& Body = TEXT(""));

	/** Create PUT request */
	static FPalantirRequest Put(const FString& URL, const FString& Body = TEXT(""));

	/** Create DELETE request */
	static FPalantirRequest Delete(const FString& URL);

	/** Create GraphQL request (POST with query) */
	static FPalantirRequest GraphQL(const FString& URL, const FString& Query, const TMap<FString, FString>& Variables = {});

	// Builder methods (fluent API)
	FPalantirRequest& WithHeader(const FString& Key, const FString& Value);
	FPalantirRequest& WithTimeout(float TimeoutSeconds);
	FPalantirRequest& WithRetry(int32 MaxRetries, float RetryDelaySeconds = 1.0f);
	FPalantirRequest& ExpectStatus(int32 StatusCode);
	FPalantirRequest& ExpectStatusRange(int32 MinStatus, int32 MaxStatus);
	FPalantirRequest& ExpectHeader(const FString& Key, const FString& Value);
	FPalantirRequest& ExpectJSON(const FString& JSONPath, const FString& ExpectedValue);
	FPalantirRequest& ExpectBodyContains(const FString& Substring);

	/** Execute request synchronously (blocks until complete or timeout) */
	FPalantirResponse ExecuteBlocking();

	/** Execute request asynchronously with callback */
	void ExecuteAsync(TFunction<void(const FPalantirResponse&)> OnComplete);

private:
	FPalantirRequest(const FString& InURL, const FString& InVerb, const FString& InBody = TEXT(""));

	FString URL;
	FString Verb;
	FString Body;
	TMap<FString, FString> Headers;
	float TimeoutSeconds;
	int32 MaxRetries;
	float RetryDelaySeconds;

	// Expectations for validation
	TOptional<int32> ExpectedStatus;
	TOptional<TPair<int32, int32>> ExpectedStatusRange;
	TMap<FString, FString> ExpectedHeaders;
	TMap<FString, FString> ExpectedJSONValues;
	TArray<FString> ExpectedBodySubstrings;

	/** Internal: Create HTTP request with trace headers */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateHttpRequest() const;

	/** Internal: Validate response against expectations */
	bool ValidateResponse(const FPalantirResponse& Response, FString& OutError) const;
};

/**
 * Convenience macros for common API testing patterns.
 */

/** Assert GET request returns 200 OK */
#define PALANTIR_ASSERT_GET_OK(URL) \
	do { \
		FPalantirResponse Res = FPalantirRequest::Get(URL).ExpectStatus(200).ExecuteBlocking(); \
		if (!Res.IsSuccess()) { \
			UE_LOG(LogPalantirTrace, Error, TEXT("GET %s failed: %d"), *URL, Res.StatusCode); \
			return false; \
		} \
	} while(0)

/** Assert POST request returns 201 Created */
#define PALANTIR_ASSERT_POST_CREATED(URL, BODY) \
	do { \
		FPalantirResponse Res = FPalantirRequest::Post(URL, BODY).ExpectStatus(201).ExecuteBlocking(); \
		if (Res.StatusCode != 201) { \
			UE_LOG(LogPalantirTrace, Error, TEXT("POST %s failed: %d"), *URL, Res.StatusCode); \
			return false; \
		} \
	} while(0)

/** Assert API health check endpoint responds */
#define PALANTIR_ASSERT_HEALTH_CHECK(URL) \
	do { \
		FPalantirResponse Res = FPalantirRequest::Get(URL).WithTimeout(5.0f).ExpectStatusRange(200, 299).ExecuteBlocking(); \
		if (!Res.IsSuccess()) { \
			UE_LOG(LogPalantirTrace, Error, TEXT("Health check %s failed: %d"), *URL, Res.StatusCode); \
			return false; \
		} \
	} while(0)
