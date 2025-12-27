#include "PalantirTrace.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogPalantirTrace, Display, All);

// Thread-local storage for trace context (avoid DLL export issues with static thread_local)
namespace FPalantirTraceLocal
{
	thread_local FString CurrentTraceID;
	thread_local TArray<TPair<double, FString>> CurrentBreadcrumbs;
	thread_local double TraceStartTime = 0.0;
}

// Static accessor functions to avoid DLL export issues
FString& FPalantirTrace::GetCurrentTraceIDRef()
{
	return FPalantirTraceLocal::CurrentTraceID;
}

TArray<TPair<double, FString>>& FPalantirTrace::GetBreadcrumbsRef()
{
	return FPalantirTraceLocal::CurrentBreadcrumbs;
}

double& FPalantirTrace::GetTraceStartTimeRef()
{
	return FPalantirTraceLocal::TraceStartTime;
}

FString FPalantirTrace::GenerateTraceID()
{
	// Format: "nexus-test-<UUID>"
	// Example: "nexus-test-a3f2e1d4-7c9b-4f2a-9e8d-3c5b1a2f0d7e"
	FGuid UniqueID = FGuid::NewGuid();
	return FString::Printf(TEXT("nexus-test-%s"), *UniqueID.ToString(EGuidFormats::DigitsWithHyphens).ToLower());
}

void FPalantirTrace::SetCurrentTraceID(const FString& TraceID)
{
	GetCurrentTraceIDRef() = TraceID;
	GetTraceStartTimeRef() = FPlatformTime::Seconds();
	GetBreadcrumbsRef().Empty();
	
	UE_LOG(LogPalantirTrace, Log, TEXT("Trace started: %s"), *TraceID);
}

FString FPalantirTrace::GetCurrentTraceID()
{
	return GetCurrentTraceIDRef();
}

void FPalantirTrace::Clear()
{
	FString& TraceID = GetCurrentTraceIDRef();
	if (!TraceID.IsEmpty())
	{
		UE_LOG(LogPalantirTrace, Log, TEXT("Trace ended: %s (duration: %.2fs)"), 
			*TraceID, FPlatformTime::Seconds() - GetTraceStartTimeRef());
	}
	TraceID.Empty();
	GetBreadcrumbsRef().Empty();
	GetTraceStartTimeRef() = 0.0;
}

void FPalantirTrace::AddBreadcrumb(const FString& EventName, const FString& Details)
{
	FString& TraceID = GetCurrentTraceIDRef();
	if (TraceID.IsEmpty())
	{
		return;  // No active trace
	}

	double Timestamp = FPlatformTime::Seconds() - GetTraceStartTimeRef();
	FString Breadcrumb = FString::Printf(
		TEXT("[%.3fs] %s: %s"),
		Timestamp,
		*EventName,
		Details.IsEmpty() ? TEXT("") : *Details
	);
	GetBreadcrumbsRef().Add(TPair<double, FString>(Timestamp, Breadcrumb));

	UE_LOG(LogPalantirTrace, Verbose, TEXT("[%s] %s"), *TraceID, *Breadcrumb);
}

TArray<TPair<double, FString>> FPalantirTrace::GetBreadcrumbs()
{
	return GetBreadcrumbsRef();
}

FString FPalantirTrace::ExportToJSON()
{
	TSharedPtr<FJsonObject> JsonRoot = MakeShareable(new FJsonObject());
	JsonRoot->SetStringField(TEXT("trace_id"), GetCurrentTraceIDRef());
	JsonRoot->SetNumberField(TEXT("start_time"), GetTraceStartTimeRef());
	JsonRoot->SetNumberField(TEXT("duration_seconds"), FPlatformTime::Seconds() - GetTraceStartTimeRef());

	TArray<TSharedPtr<FJsonValue>> BreadcrumbArray;
	for (const auto& Breadcrumb : GetBreadcrumbsRef())
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

