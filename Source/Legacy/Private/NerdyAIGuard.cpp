// Deprecated stub for NerdyAIGuard — moved to `ai-castle` repository.
// Implementations here are no-ops that log a warning.

#include "Misc/OutputDevice.h"
#include "Misc/Paths.h"

class FNerdyAIGuard
{
public:
    static bool IsAIOutputLegal(const FString& AIResponse)
    {
        UE_LOG(LogTemp, Warning, TEXT("FNerdyAIGuard::IsAIOutputLegal called on deprecated stub. Use ai-castle repo for full checks."));
        // Conservative default: treat unknown AI output as potentially unsafe
        return false;
    }

    static void GenerateTestFromAI(const FString& Prompt)
    {
        UE_LOG(LogTemp, Warning, TEXT("FNerdyAIGuard::GenerateTestFromAI called on deprecated stub. Use ai-castle repo for real generation."));
        // No-op
    }
};