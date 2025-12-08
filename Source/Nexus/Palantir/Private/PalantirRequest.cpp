#include "PalantirRequest.h"
#include "PalantirTrace.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"

//------------------------------------------------------------------------------
// FPalantirResponse Implementation
//------------------------------------------------------------------------------

TSharedPtr<FJsonObject> FPalantirResponse::GetJSON() const
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return JsonObject;
	}
	
	return nullptr;
}

FString FPalantirResponse::GetJSONValue(const FString& JSONPath) const
{
	TSharedPtr<FJsonObject> JsonObject = GetJSON();
	if (!JsonObject.IsValid())
	{
		return TEXT("");
	}

	// Simplified JSONPath: support dot notation like "user.name"
	TArray<FString> PathParts;
	JSONPath.ParseIntoArray(PathParts, TEXT("."));

	TSharedPtr<FJsonValue> CurrentValue;
	TSharedPtr<FJsonObject> CurrentObject = JsonObject;

	for (int32 i = 0; i < PathParts.Num(); ++i)
	{
		if (!CurrentObject.IsValid())
		{
			return TEXT("");
		}

		const FString& Part = PathParts[i];
		CurrentValue = CurrentObject->TryGetField(Part);

		if (!CurrentValue.IsValid())
		{
			return TEXT("");
		}

		// If not the last part, navigate deeper
		if (i < PathParts.Num() - 1)
		{
			CurrentObject = CurrentValue->AsObject();
		}
	}

	// Return final value as string
	if (CurrentValue.IsValid())
	{
		FString OutString;
		if (CurrentValue->TryGetString(OutString))
		{
			return OutString;
		}
		// Fallback: convert to string representation
		return CurrentValue->AsString();
	}

	return TEXT("");
}

bool FPalantirResponse::Validate(FString& OutError) const
{
	// Basic validation: just check if successful
	if (!IsSuccess())
	{
		OutError = FString::Printf(TEXT("HTTP %d"), StatusCode);
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
// FPalantirRequest Implementation
//------------------------------------------------------------------------------

FPalantirRequest::FPalantirRequest(const FString& InURL, const FString& InVerb, const FString& InBody)
	: URL(InURL)
	, Verb(InVerb)
	, Body(InBody)
	, TimeoutSeconds(30.0f)
	, MaxRetries(0)
	, RetryDelaySeconds(1.0f)
{
	// Default headers
	Headers.Add(TEXT("Content-Type"), TEXT("application/json"));
}

FPalantirRequest FPalantirRequest::Get(const FString& URL)
{
	return FPalantirRequest(URL, TEXT("GET"));
}

FPalantirRequest FPalantirRequest::Post(const FString& URL, const FString& Body)
{
	return FPalantirRequest(URL, TEXT("POST"), Body);
}

FPalantirRequest FPalantirRequest::Put(const FString& URL, const FString& Body)
{
	return FPalantirRequest(URL, TEXT("PUT"), Body);
}

FPalantirRequest FPalantirRequest::Delete(const FString& URL)
{
	return FPalantirRequest(URL, TEXT("DELETE"));
}

FPalantirRequest FPalantirRequest::GraphQL(const FString& URL, const FString& Query, const TMap<FString, FString>& Variables)
{
	// Build GraphQL request body
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("query"), Query);

	if (Variables.Num() > 0)
	{
		TSharedPtr<FJsonObject> VarsObject = MakeShared<FJsonObject>();
		for (const auto& Pair : Variables)
		{
			VarsObject->SetStringField(Pair.Key, Pair.Value);
		}
		JsonObject->SetObjectField(TEXT("variables"), VarsObject);
	}

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return FPalantirRequest(URL, TEXT("POST"), Body);
}

FPalantirRequest& FPalantirRequest::WithHeader(const FString& Key, const FString& Value)
{
	Headers.Add(Key, Value);
	return *this;
}

FPalantirRequest& FPalantirRequest::WithTimeout(float InTimeoutSeconds)
{
	TimeoutSeconds = InTimeoutSeconds;
	return *this;
}

FPalantirRequest& FPalantirRequest::WithRetry(int32 InMaxRetries, float InRetryDelaySeconds)
{
	MaxRetries = InMaxRetries;
	RetryDelaySeconds = InRetryDelaySeconds;
	return *this;
}

FPalantirRequest& FPalantirRequest::ExpectStatus(int32 StatusCode)
{
	ExpectedStatus = StatusCode;
	return *this;
}

FPalantirRequest& FPalantirRequest::ExpectStatusRange(int32 MinStatus, int32 MaxStatus)
{
	ExpectedStatusRange = TPair<int32, int32>(MinStatus, MaxStatus);
	return *this;
}

FPalantirRequest& FPalantirRequest::ExpectHeader(const FString& Key, const FString& Value)
{
	ExpectedHeaders.Add(Key, Value);
	return *this;
}

FPalantirRequest& FPalantirRequest::ExpectJSON(const FString& JSONPath, const FString& ExpectedValue)
{
	ExpectedJSONValues.Add(JSONPath, ExpectedValue);
	return *this;
}

FPalantirRequest& FPalantirRequest::ExpectBodyContains(const FString& Substring)
{
	ExpectedBodySubstrings.Add(Substring);
	return *this;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FPalantirRequest::CreateHttpRequest() const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(URL);
	Request->SetVerb(Verb);
	Request->SetTimeout(TimeoutSeconds);

	if (!Body.IsEmpty())
	{
		Request->SetContentAsString(Body);
	}

	// Set all headers
	for (const auto& Pair : Headers)
	{
		Request->SetHeader(Pair.Key, Pair.Value);
	}

	// Inject trace ID from Palantír context
	FString TraceID = FPalantirTrace::GetCurrentTraceID();
	if (!TraceID.IsEmpty())
	{
		Request->SetHeader(TEXT("X-Trace-ID"), TraceID);
		Request->SetHeader(TEXT("User-Agent"), FString::Printf(TEXT("NexusTest/%s"), *TraceID));
		
		// Log breadcrumb for network request
		PALANTIR_BREADCRUMB(TEXT("HttpRequest"), FString::Printf(TEXT("%s %s"), *Verb, *URL));
	}

	return Request;
}

bool FPalantirRequest::ValidateResponse(const FPalantirResponse& Response, FString& OutError) const
{
	// Validate status code
	if (ExpectedStatus.IsSet() && Response.StatusCode != ExpectedStatus.GetValue())
	{
		OutError = FString::Printf(TEXT("Expected status %d, got %d"), ExpectedStatus.GetValue(), Response.StatusCode);
		return false;
	}

	// Validate status range
	if (ExpectedStatusRange.IsSet())
	{
		int32 Min = ExpectedStatusRange.GetValue().Key;
		int32 Max = ExpectedStatusRange.GetValue().Value;
		if (Response.StatusCode < Min || Response.StatusCode > Max)
		{
			OutError = FString::Printf(TEXT("Expected status in range [%d, %d], got %d"), Min, Max, Response.StatusCode);
			return false;
		}
	}

	// Validate headers
	for (const auto& Pair : ExpectedHeaders)
	{
		const FString* ActualValue = Response.Headers.Find(Pair.Key);
		if (!ActualValue || *ActualValue != Pair.Value)
		{
			OutError = FString::Printf(TEXT("Expected header %s=%s, got %s"), *Pair.Key, *Pair.Value, ActualValue ? **ActualValue : TEXT("(missing)"));
			return false;
		}
	}

	// Validate JSON values
	for (const auto& Pair : ExpectedJSONValues)
	{
		FString ActualValue = Response.GetJSONValue(Pair.Key);
		if (ActualValue != Pair.Value)
		{
			OutError = FString::Printf(TEXT("Expected JSON path %s=%s, got %s"), *Pair.Key, *Pair.Value, *ActualValue);
			return false;
		}
	}

	// Validate body substrings
	for (const FString& Substring : ExpectedBodySubstrings)
	{
		if (!Response.Body.Contains(Substring))
		{
			OutError = FString::Printf(TEXT("Expected body to contain: %s"), *Substring);
			return false;
		}
	}

	return true;
}

FPalantirResponse FPalantirRequest::ExecuteBlocking()
{
	FPalantirResponse Response;
	Response.TraceID = FPalantirTrace::GetCurrentTraceID();

	int32 Attempt = 0;
	bool bSuccess = false;
	FString ValidationError;

	double StartTime = FPlatformTime::Seconds();

	while (Attempt <= MaxRetries && !bSuccess)
	{
		if (Attempt > 0)
		{
			// Exponential backoff
			float DelaySeconds = RetryDelaySeconds * FMath::Pow(2.0f, Attempt - 1);
			UE_LOG(LogPalantirTrace, Warning, TEXT("Retrying %s %s (attempt %d/%d) after %.1fs"), *Verb, *URL, Attempt + 1, MaxRetries + 1, DelaySeconds);
			FPlatformProcess::Sleep(DelaySeconds);
		}

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateHttpRequest();

		// Use event for blocking wait
		FEvent* CompletionEvent = FPlatformProcess::GetSynchEventFromPool(false);
		bool bRequestCompleted = false;

		Request->OnProcessRequestComplete().BindLambda([&Response, &bRequestCompleted, CompletionEvent](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
		{
			if (bConnectedSuccessfully && Res.IsValid())
			{
				Response.StatusCode = Res->GetResponseCode();
				Response.Body = Res->GetContentAsString();

				// Extract headers
				for (const FString& HeaderName : Res->GetAllHeaders())
				{
					FString Key, Value;
					if (HeaderName.Split(TEXT(":"), &Key, &Value))
					{
						Response.Headers.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
					}
				}
			}
			else
			{
				Response.StatusCode = 0; // Connection failed
				Response.Body = TEXT("");
			}

			bRequestCompleted = true;
			CompletionEvent->Trigger();
		});

		if (!Request->ProcessRequest())
		{
			UE_LOG(LogPalantirTrace, Error, TEXT("Failed to start HTTP request: %s %s"), *Verb, *URL);
			Response.StatusCode = 0;
			FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
			break;
		}

		// Wait for completion or timeout
		CompletionEvent->Wait(static_cast<uint32>(TimeoutSeconds * 1000.0f));
		FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);

		Response.DurationMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

		// Log response breadcrumb
		if (!Response.TraceID.IsEmpty())
		{
			PALANTIR_BREADCRUMB(TEXT("HttpResponse"), FString::Printf(TEXT("%d in %.1fms"), Response.StatusCode, Response.DurationMs));
		}

		// Validate response
		if (ValidateResponse(Response, ValidationError))
		{
			bSuccess = true;
		}
		else
		{
			UE_LOG(LogPalantirTrace, Warning, TEXT("Validation failed: %s"), *ValidationError);
		}

		++Attempt;
	}

	if (!bSuccess && !ValidationError.IsEmpty())
	{
		UE_LOG(LogPalantirTrace, Error, TEXT("Request failed after %d attempts: %s"), Attempt, *ValidationError);
	}

	return Response;
}

void FPalantirRequest::ExecuteAsync(TFunction<void(const FPalantirResponse&)> OnComplete)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateHttpRequest();
	FString TraceID = FPalantirTrace::GetCurrentTraceID();
	double StartTime = FPlatformTime::Seconds();

	Request->OnProcessRequestComplete().BindLambda([OnComplete, TraceID, StartTime, this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bConnectedSuccessfully)
	{
		FPalantirResponse Response;
		Response.TraceID = TraceID;
		Response.DurationMs = static_cast<float>((FPlatformTime::Seconds() - StartTime) * 1000.0);

		if (bConnectedSuccessfully && Res.IsValid())
		{
			Response.StatusCode = Res->GetResponseCode();
			Response.Body = Res->GetContentAsString();

			// Extract headers
			for (const FString& HeaderName : Res->GetAllHeaders())
			{
				FString Key, Value;
				if (HeaderName.Split(TEXT(":"), &Key, &Value))
				{
					Response.Headers.Add(Key.TrimStartAndEnd(), Value.TrimStartAndEnd());
				}
			}
		}
		else
		{
			Response.StatusCode = 0;
			Response.Body = TEXT("");
		}

		// Log response breadcrumb
		if (!TraceID.IsEmpty())
		{
			PALANTIR_BREADCRUMB(TEXT("HttpResponse"), FString::Printf(TEXT("%d in %.1fms"), Response.StatusCode, Response.DurationMs));
		}

		// Call user callback
		OnComplete(Response);
	});

	Request->ProcessRequest();
}
