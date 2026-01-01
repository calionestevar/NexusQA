#pragma once

#include "CoreMinimal.h"

class FNexusConsoleCommands
{
public:
	static void Register();

private:
	static void OnRunTests(const TArray<FString>& Args);
};
