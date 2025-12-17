#include "FringeNetwork.h"
#include "HttpModule.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformAtomics.h"
#include "Templates/SharedPointer.h"
#include "Containers/List.h"

void UFringeNetwork::TestParallelRealms(const TArray<FString>& RegionURLs)
{
    UE_LOG(LogTemp, Display, TEXT("FRINGE NETWORK: Testing %d parallel realms"), RegionURLs.Num());

    if (RegionURLs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("FRINGE NETWORK: No regions provided to TestParallelRealms"));
        return;
    }

    // Shared counters - use thread-safe atomics
    TSharedRef<FThreadSafeCounter> SuccessCount = MakeShared<FThreadSafeCounter>(0);
    TSharedRef<FThreadSafeCounter> Remaining = MakeShared<FThreadSafeCounter>(RegionURLs.Num());

    // Completion event to wait for all callbacks (or timeout).
    FEvent* CompletionEvent = FPlatformProcess::GetSynchEventFromPool(false);

    for (const FString& URL : RegionURLs)
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->OnProcessRequestComplete().BindLambda([SuccessCount, Remaining, CompletionEvent, URL](FHttpRequestPtr, FHttpResponsePtr Res, bool bSuccess)
        {
            if (bSuccess && Res.IsValid() && Res->GetResponseCode() == 200)
            {
                SuccessCount->Increment();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PARALLEL REALM FAILURE: %s"), *URL);
            }

            if (Remaining->Decrement() <= 0)
            {
                CompletionEvent->Trigger();
            }
        });
        Request->SetURL(URL);
        Request->SetVerb("HEAD");
        Request->ProcessRequest();
    }

    // Wait up to 5s for all responses (non-infinite to avoid hanging CI). This blocks
    // the calling thread; tests should run this on a worker thread in CI.
    const int32 TimeoutMs = 5000;
    CompletionEvent->Wait(TimeoutMs);
    FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);

    float SyncRate = (float)SuccessCount->GetValue() / RegionURLs.Num();
    UE_LOG(LogTemp, Display, TEXT("FRINGE NETWORK: Realm synchronization: %.0f%%"), SyncRate * 100);
}