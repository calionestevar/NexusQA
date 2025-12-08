#pragma once
#include "Commandlets/Commandlet.h"
#include "AsgardCommandlet.generated.h"

UCLASS()
class UAsgardCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    virtual int32 Main(const FString& Params) override;
};