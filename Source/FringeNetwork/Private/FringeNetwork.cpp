#include "FringeNetwork.h"
#include "CortexiphanInjector.h"
#include "ObserverNetworkDashboard.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UFringeNetwork::ActivateObserverNetwork()
{
	UE_LOG(LogTemp, Warning, TEXT("🌀 FRINGE NETWORK ACTIVATED — OBSERVER PROTOCOL INITIALIZED"));

	// Initialize HTTP module for observer communications
	FHttpModule& HttpModule = FHttpModule::Get();
	if (!HttpModule.IsHttpEnabled())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ HTTP Module not available for Observer Network"));
		return;
	}

	// Activate the observer network dashboard for real-time monitoring
	UObserverNetworkDashboard::InitializeDashboard();

	// Log activation
	UE_LOG(LogTemp, Display, TEXT("✅ Observer Network active — Monitoring all test vectors"));
}

void UFringeNetwork::RunObserverNetworkTests(const FString& PrimaryServer)
{
	if (PrimaryServer.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No primary server specified for Observer Network tests"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("🔍 OBSERVER NETWORK TEST SEQUENCE INITIATED — TARGET: %s"), *PrimaryServer);

	// Initialize HTTP module
	FHttpModule& HttpModule = FHttpModule::Get();
	if (!HttpModule.IsHttpEnabled())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ HTTP Module not available"));
		return;
	}

	// Create HTTP request to primary server
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->SetURL(PrimaryServer);

	// Set completion callback
	HttpRequest->OnProcessRequestComplete().BindLambda([PrimaryServer](
		FHttpRequestPtr Request,
		FHttpResponsePtr Response,
		bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			int32 ResponseCode = Response->GetResponseCode();
			FString ResponseBody = Response->GetContentAsString();
			
			UE_LOG(LogTemp, Display, TEXT("✅ Observer Network Test Complete — Server: %s, Status: %d, Size: %d bytes"),
				*PrimaryServer, ResponseCode, ResponseBody.Len());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("❌ Observer Network Test Failed — Server: %s"), *PrimaryServer);
		}
	});

	// Execute the request
	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Display, TEXT("📡 Observer Network request dispatched to %s"), *PrimaryServer);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Failed to dispatch Observer Network request"));
	}
}

void UFringeNetwork::TestParallelRealms(const TArray<FString>& RegionURLs)
{
	if (RegionURLs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  No region URLs provided for parallel realm testing"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("🌀 PARALLEL REALM TEST INITIATED — Testing %d regions"), RegionURLs.Num());

	FHttpModule& HttpModule = FHttpModule::Get();
	if (!HttpModule.IsHttpEnabled())
	{
		UE_LOG(LogTemp, Error, TEXT("❌ HTTP Module not available for parallel realm tests"));
		return;
	}

	// Test each region in parallel
	for (const FString& RegionURL : RegionURLs)
	{
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->SetURL(RegionURL);
		HttpRequest->SetTimeout(5.0f);

		HttpRequest->OnProcessRequestComplete().BindLambda([RegionURL](
			FHttpRequestPtr Request,
			FHttpResponsePtr Response,
			bool bWasSuccessful)
		{
			if (bWasSuccessful && Response.IsValid())
			{
				UE_LOG(LogTemp, Display, TEXT("✅ Parallel Realm Test Passed — Region: %s"), *RegionURL);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("⚠️  Parallel Realm Test Failed — Region: %s"), *RegionURL);
			}
		});

		if (!HttpRequest->ProcessRequest())
		{
			UE_LOG(LogTemp, Error, TEXT("❌ Failed to dispatch parallel realm test for %s"), *RegionURL);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("📡 Parallel realm tests dispatched for %d regions"), RegionURLs.Num());
}

void UFringeNetwork::InjectCortexiphanChaos(float DurationSeconds)
{
	if (DurationSeconds <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️  Invalid duration for Cortexiphan chaos: %.2f seconds"), DurationSeconds);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("⚡ CORTEXIPHAN INJECTION SEQUENCE INITIATED — DURATION: %.1f SECONDS"), DurationSeconds);

	// Get the current world
	if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
	{
		// Inject chaos through the CortexiphanInjector
		UCortexiphanInjector::InjectChaos(World, DurationSeconds, 1.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ No valid world for Cortexiphan injection"));
	}
}
