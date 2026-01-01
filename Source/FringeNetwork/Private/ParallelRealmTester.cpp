#include "FringeNetwork.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
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
    TSharedPtr<std::atomic<bool>> bEventValidPtr = MakeShared<std::atomic<bool>>(true);
    TSharedPtr<std::atomic<bool>> bAllRequestsCompletedPtr = MakeShared<std::atomic<bool>>(false);

    for (const FString& URL : RegionURLs)
    {
        // Capture URL by value to avoid dangling reference
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->OnProcessRequestComplete().BindLambda([SuccessCount, Remaining, CompletionEvent, bEventValidPtr, bAllRequestsCompletedPtr, URL](FHttpRequestPtr, FHttpResponsePtr Res, bool bSuccess)
        {
            if (bSuccess && Res.IsValid() && Res->GetResponseCode() == 200)
            {
                SuccessCount->Increment();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PARALLEL REALM FAILURE: %s"), *URL);
            }

            int32 RemainingCount = Remaining->Decrement();
            if (RemainingCount <= 0)
            {
                // Mark all requests as completed before triggering event
                bAllRequestsCompletedPtr->store(true, std::memory_order_release);
                
                // Only trigger if event is still valid
                if (bEventValidPtr && *bEventValidPtr && CompletionEvent)
                {
                    CompletionEvent->Trigger();
                }
            }
        });
        Request->SetURL(URL);
        Request->SetVerb("HEAD");
        Request->ProcessRequest();
    }

    // Wait up to 5s for all responses (non-infinite to avoid hanging CI). This blocks
    // the calling thread; tests should run this on a worker thread in CI.
    const int32 TimeoutMs = 5000;
    if (CompletionEvent)
    {
        CompletionEvent->Wait(TimeoutMs);
    }
    
    // Spin-wait briefly for completion flag to be set
    uint32 SpinWaitMs = 0;
    const uint32 MaxSpinWaitMs = 10;
    while (bAllRequestsCompletedPtr && !bAllRequestsCompletedPtr->load(std::memory_order_acquire) && SpinWaitMs < MaxSpinWaitMs)
    {
        FPlatformProcess::Sleep(0.001f);
        SpinWaitMs++;
    }
    
    // Mark event as invalid before returning to pool
    if (bEventValidPtr)
    {
        *bEventValidPtr = false;
    }
    
    if (CompletionEvent)
    {
        FPlatformProcess::ReturnSynchEventToPool(CompletionEvent);
    }

    float SyncRate = (float)SuccessCount->GetValue() / RegionURLs.Num();
    UE_LOG(LogTemp, Display, TEXT("FRINGE NETWORK: Realm synchronization: %.0f%%"), SyncRate * 100);
}