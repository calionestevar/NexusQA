#include "FringeNetwork.h"
#include "HttpModule.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformAtomics.h"
#include "Templates/SharedPointer.h"

void UFringeNetwork::TestParallelRealms(const TArray<FString>& RegionURLs)
{
    UE_LOG(LogTemp, Display, TEXT("FRINGE NETWORK: Testing %d parallel realms"), RegionURLs.Num());

    if (RegionURLs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("FRINGE NETWORK: No regions provided to TestParallelRealms"));
        return;
    }

    // Shared counters on the heap so callbacks remain valid after this function returns.
    TSharedRef<int32, ESPMode::ThreadSafe> SuccessCount = MakeShared<int32, ESPMode::ThreadSafe>(0);
    TSharedRef<int32, ESPMode::ThreadSafe> Remaining = MakeShared<int32, ESPMode::ThreadSafe>(RegionURLs.Num());

    // Completion event to wait for all callbacks (or timeout).
    FEvent* CompletionEvent = FPlatformProcess::GetSynchEventFromPool(false);

    for (const FString& URL : RegionURLs)
    {
        TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
        Request->OnProcessRequestComplete().BindLambda([SuccessCount, Remaining, CompletionEvent, URL](FHttpRequestPtr, FHttpResponsePtr Res, bool bSuccess)
        {
            if (bSuccess && Res.IsValid() && Res->GetResponseCode() == 200)
            {
                FPlatformAtomics::InterlockedIncrement((volatile int32*)SuccessCount.Get());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("PARALLEL REALM FAILURE: %s"), *URL);
            }

            if (FPlatformAtomics::InterlockedDecrement((volatile int32*)Remaining.Get()) <= 0)
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

    float SyncRate = (float)(*SuccessCount) / RegionURLs.Num();
    TestTrue(TEXT("90%+ parallel realms synchronized"), SyncRate >= 0.9f);
    UE_LOG(LogTemp, Display, TEXT("FRINGE NETWORK: Realm synchronization: %.0f%%"), SyncRate * 100);
}