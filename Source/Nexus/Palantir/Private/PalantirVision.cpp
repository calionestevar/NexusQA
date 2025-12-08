#include "PalantirVision.h"
#include "PalantirTrace.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogPalantirVision, Display, All);

FAssertionContext::FAssertionContext(const FString& InCondition, const FString& InFile, int32 InLine)
	: Condition(InCondition), FilePath(InFile), LineNumber(InLine)
{
}

FAssertionContext& FAssertionContext::WithContext(const FString& Key, const FString& Value)
{
	ContextData.Add(Key, Value);
	return *this;
}

FAssertionContext& FAssertionContext::WithContext(const FString& Key, float Value)
{
	ContextData.Add(Key, FString::Printf(TEXT("%.2f"), Value));
	return *this;
}

FAssertionContext& FAssertionContext::WithContext(const FString& Key, int32 Value)
{
	ContextData.Add(Key, FString::FromInt(Value));
	return *this;
}

FAssertionContext& FAssertionContext::WithHint(const FString& Hint)
{
	HintText = Hint;
	return *this;
}

FAssertionContext& FAssertionContext::WithPerformanceData()
{
	bIncludePerformanceData = true;
	
	// Note: Actual ArgusLens integration requires runtime check to avoid circular dependency
	// Performance data will be attached if ArgusLens module is loaded
	// Implementation: Check if UArgusLens class exists, call GetCurrentPerformanceSnapshot()
	
	return *this;
}

FString FAssertionContext::BuildDetailedMessage() const
{
	FString DetailedMessage = FString::Printf(
		TEXT("Assertion Failed: %s\n"),
		*Condition
	);

	DetailedMessage += FString::Printf(TEXT("Location: %s(%d)\n"), *FilePath, LineNumber);
	DetailedMessage += FString::Printf(TEXT("Trace ID: %s\n"), *FPalantirTrace::GetCurrentTraceID());

	if (!HintText.IsEmpty())
	{
		DetailedMessage += FString::Printf(TEXT("Hint: %s\n"), *HintText);
	}

	if (ContextData.Num() > 0)
	{
		DetailedMessage += TEXT("Context:\n");
		for (const auto& ContextPair : ContextData)
		{
			DetailedMessage += FString::Printf(TEXT("  %s: %s\n"), *ContextPair.Key, *ContextPair.Value);
		}
	}

	if (bIncludePerformanceData)
	{
		// TODO: Integrate with ArgusLens to fetch current perf metrics
		// DetailedMessage += FString::Printf(TEXT("  FPS: %.1f\n"), GetCurrentFPS());
		// DetailedMessage += FString::Printf(TEXT("  Memory: %d MB\n"), GetMemoryMB());
	}

	return DetailedMessage;
}

FString FAssertionContext::ExportToJSON() const
{
	TSharedPtr<FJsonObject> JsonRoot = MakeShareable(new FJsonObject());
	JsonRoot->SetStringField(TEXT("assertion"), Condition);
	JsonRoot->SetStringField(TEXT("file"), FilePath);
	JsonRoot->SetNumberField(TEXT("line"), LineNumber);
	JsonRoot->SetStringField(TEXT("trace_id"), FPalantirTrace::GetCurrentTraceID());
	JsonRoot->SetBoolField(TEXT("passed"), bConditionMet);

	if (!HintText.IsEmpty())
	{
		JsonRoot->SetStringField(TEXT("hint"), HintText);
	}

	TSharedPtr<FJsonObject> ContextObj = MakeShareable(new FJsonObject());
	for (const auto& ContextPair : ContextData)
	{
		ContextObj->SetStringField(ContextPair.Key, ContextPair.Value);
	}
	JsonRoot->SetObjectField(TEXT("context"), ContextObj);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonRoot.ToSharedRef(), Writer);

	return JsonString;
}

bool FAssertionContext::ExecuteOrFail()
{
	if (!bConditionMet)
	{
		FString DetailedMessage = BuildDetailedMessage();
		UE_LOG(LogPalantirVision, Error, TEXT("%s"), *DetailedMessage);
		
		// Also log as JSON for structured analysis
		UE_LOG(LogPalantirVision, Verbose, TEXT("JSON: %s"), *ExportToJSON());

		FPalantirTrace::AddBreadcrumb(TEXT("AssertionFailed"), Condition);
		check(false);  // Fail the test
		return false;
	}

	FPalantirTrace::AddBreadcrumb(TEXT("AssertionPassed"), Condition);
	return true;
}

