#include "PalantirTrace.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogPalantirTrace, Display, All);

// Thread-local storage for trace context
thread_local FString FPalantirTrace::CurrentTraceID;
thread_local TArray<TPair<double, FString>> FPalantirTrace::CurrentBreadcrumbs;
thread_local double FPalantirTrace::TraceStartTime = 0.0;

FString FPalantirTrace::GenerateTraceID()
{
	// Format: "nexus-test-<UUID>"
	// Example: "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e"
	FGuid UniqueID = FGuid::NewGuid();
	return FString::Printf(TEXT("nexus-test-%s"), *UniqueID.ToString(EGuidFormats::DigitsWithHyphens).ToLower());
}

void FPalantirTrace::SetCurrentTraceID(const FString& TraceID)
{
	CurrentTraceID = TraceID;
	TraceStartTime = FPlatformTime::Seconds();
	CurrentBreadcrumbs.Empty();
	
	UE_LOG(LogPalantirTrace, Log, TEXT("Trace started: %s"), *TraceID);
}

FString FPalantirTrace::GetCurrentTraceID()
{
	return CurrentTraceID;
}

void FPalantirTrace::Clear()
{
	if (!CurrentTraceID.IsEmpty())
	{
		UE_LOG(LogPalantirTrace, Log, TEXT("Trace ended: %s (duration: %.2fs)"), 
			*CurrentTraceID, FPlatformTime::Seconds() - TraceStartTime);
	}
	CurrentTraceID.Empty();
	CurrentBreadcrumbs.Empty();
	TraceStartTime = 0.0;
}

void FPalantirTrace::AddBreadcrumb(const FString& EventName, const FString& Details)
{
	if (CurrentTraceID.IsEmpty())
	{
		return;  // No active trace
	}

	double Timestamp = FPlatformTime::Seconds() - TraceStartTime;
	FString Breadcrumb = FString::Printf(
		TEXT("[%.3fs] %s: %s"),
		Timestamp,
		*EventName,
		Details.IsEmpty() ? TEXT("") : *Details
	);
	CurrentBreadcrumbs.Add(TPair<double, FString>(Timestamp, Breadcrumb));

	UE_LOG(LogPalantirTrace, Verbose, TEXT("[%s] %s"), *CurrentTraceID, *Breadcrumb);
}

TArray<TPair<double, FString>> FPalantirTrace::GetBreadcrumbs()
{
	return CurrentBreadcrumbs;
}

FString FPalantirTrace::ExportToJSON()
{
	TSharedPtr<FJsonObject> JsonRoot = MakeShareable(new FJsonObject());
	JsonRoot->SetStringField(TEXT("trace_id"), CurrentTraceID);
	JsonRoot->SetNumberField(TEXT("start_time"), TraceStartTime);
	JsonRoot->SetNumberField(TEXT("duration_seconds"), FPlatformTime::Seconds() - TraceStartTime);

	TArray<TSharedPtr<FJsonValue>> BreadcrumbArray;
	for (const auto& Breadcrumb : CurrentBreadcrumbs)
	{
		TSharedPtr<FJsonObject> BreadcrumbObj = MakeShareable(new FJsonObject());
		BreadcrumbObj->SetNumberField(TEXT("timestamp"), Breadcrumb.Key);
		BreadcrumbObj->SetStringField(TEXT("event"), Breadcrumb.Value);
		BreadcrumbArray.Add(MakeShareable(new FJsonValueObject(BreadcrumbObj)));
	}
	JsonRoot->SetArrayField(TEXT("breadcrumbs"), BreadcrumbArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonRoot.ToSharedRef(), Writer);

	return JsonString;
}

// FPalantirTraceGuard Implementation
FPalantirTraceGuard::FPalantirTraceGuard()
{
	TraceID = FPalantirTrace::GenerateTraceID();
	FPalantirTrace::SetCurrentTraceID(TraceID);
}

FPalantirTraceGuard::~FPalantirTraceGuard()
{
	FPalantirTrace::Clear();
}

