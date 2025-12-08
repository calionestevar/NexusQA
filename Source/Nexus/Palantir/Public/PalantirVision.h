#pragma once

#include "CoreMinimal.h"
#include "PalantirTrace.h"

/**
 * Enhanced assertion system for Nexus tests.
 * 
 * Inspired by Garmin's error message enhancements to Mocha/Chai assertions.
 * Provides rich context capture, fluent builder API, and automatic performance data attachment.
 * 
 * Example:
 *   NEXUS_ASSERT_GT(FPS, 30.0f)
 *       .WithContext("GPU Load", GetGPULoad())
 *       .WithContext("Memory", GetMemoryMb())
 *       .WithHint("Check ArgusLens for performance spike")
 *       .ExecuteOrFail();
 */

class NEXUS_API FAssertionContext
{
public:
	FAssertionContext(const FString& InCondition, const FString& InFile, int32 InLine);

	/**
	 * Add contextual information (e.g., GPU load, memory usage, network latency).
	 * This data is automatically included in failure messages and JSON exports.
	 */
	FAssertionContext& WithContext(const FString& Key, const FString& Value);
	FAssertionContext& WithContext(const FString& Key, float Value);
	FAssertionContext& WithContext(const FString& Key, int32 Value);

	/**
	 * Add a human-readable hint for debugging.
	 * Example: "Check for GC spikes" or "Verify network connectivity"
	 */
	FAssertionContext& WithHint(const FString& Hint);

	/**
	 * Attach performance data automatically (FPS, memory, etc.).
	 */
	FAssertionContext& WithPerformanceData();

	/**
	 * Export detailed failure information to JSON for logging/analysis.
	 */
	FString ExportToJSON() const;

	/**
	 * Execute the assertion and fail (with rich context) if condition is false.
	 */
	bool ExecuteOrFail();

	/**
	 * Check if condition is true.
	 */
	void SetCondition(bool bCondition) { bConditionMet = bCondition; }

private:
	FString Condition;
	bool bConditionMet = true;
	FString FilePath;
	int32 LineNumber;
	FString HintText;
	TMap<FString, FString> ContextData;
	bool bIncludePerformanceData = false;

	FString BuildDetailedMessage() const;
};

/**
 * Assertion builder class for fluent API.
 * 
 * Usage:
 *   AssertThat(MyValue)
 *       .IsGreaterThan(ExpectedValue)
 *       .WithContext("CurrentState", StateString)
 *       .ExecuteOrFail();
 */
template<typename T>
class TAssertionBuilder
{
public:
	TAssertionBuilder(const T& InValue, const FString& InFile, int32 InLine)
		: Value(InValue), File(InFile), Line(InLine), Context(TEXT(""), InFile, InLine)
	{
	}

	FAssertionContext& IsGreaterThan(const T& Threshold)
	{
		bool bCondition = Value > Threshold;
		Context.SetCondition(bCondition);
		if (!bCondition)
		{
			Context.WithContext(TEXT("Expected"), FString::Printf(TEXT("> %s"), *FString::FromInt(static_cast<int32>(Threshold))));
			Context.WithContext(TEXT("Actual"), FString::Printf(TEXT("%s"), *FString::FromInt(static_cast<int32>(Value))));
		}
		return Context;
	}

	FAssertionContext& IsLessThan(const T& Threshold)
	{
		bool bCondition = Value < Threshold;
		Context.SetCondition(bCondition);
		if (!bCondition)
		{
			Context.WithContext(TEXT("Expected"), FString::Printf(TEXT("< %s"), *FString::FromInt(static_cast<int32>(Threshold))));
			Context.WithContext(TEXT("Actual"), FString::Printf(TEXT("%s"), *FString::FromInt(static_cast<int32>(Value))));
		}
		return Context;
	}

	FAssertionContext& IsEqual(const T& Expected)
	{
		bool bCondition = Value == Expected;
		Context.SetCondition(bCondition);
		if (!bCondition)
		{
			Context.WithContext(TEXT("Expected"), FString::FromInt(static_cast<int32>(Expected)));
			Context.WithContext(TEXT("Actual"), FString::FromInt(static_cast<int32>(Value)));
		}
		return Context;
	}

private:
	T Value;
	FString File;
	int32 Line;
	FAssertionContext Context;
};

/**
 * NEXUS_ASSERT: Core assertion macro with full context capture.
 * 
 * Usage:
 *   NEXUS_ASSERT(Condition, TEXT("Optional message"))
 *       .WithContext("State", StateVariable)
 *       .ExecuteOrFail();
 */
#define NEXUS_ASSERT(Condition, Message) \
	FAssertionContext(TEXT(#Condition), __FILE__, __LINE__) \
		.WithContext(TEXT("Message"), Message); \
	if (!(Condition)) { /* assertion fails */ } \
	FPalantirTrace::AddBreadcrumb(TEXT("Assertion"), TEXT(#Condition))

/**
 * Convenience macros for common comparison assertions.
 * Usage: NEXUS_ASSERT_GT(Value, Threshold).WithContext(...).ExecuteOrFail();
 */
#define NEXUS_ASSERT_GT(Value, Threshold) \
	TAssertionBuilder<decltype(Value)>(Value, __FILE__, __LINE__).IsGreaterThan(Threshold)

#define NEXUS_ASSERT_LT(Value, Threshold) \
	TAssertionBuilder<decltype(Value)>(Value, __FILE__, __LINE__).IsLessThan(Threshold)

#define NEXUS_ASSERT_EQ(Value, Expected) \
	TAssertionBuilder<decltype(Value)>(Value, __FILE__, __LINE__).IsEqual(Expected)

/**
 * NEXUS_ASSERT_TRUE and NEXUS_ASSERT_FALSE for boolean conditions.
 */
#define NEXUS_ASSERT_TRUE(Condition) \
	do { \
		if (!(Condition)) { \
			FAssertionContext Context(TEXT(#Condition), __FILE__, __LINE__); \
			Context.WithHint(TEXT("Condition evaluated to false")); \
			Context.ExecuteOrFail(); \
		} \
		FPalantirTrace::AddBreadcrumb(TEXT("AssertTrue"), TEXT(#Condition)); \
	} while(false)

#define NEXUS_ASSERT_FALSE(Condition) \
	do { \
		if (Condition) { \
			FAssertionContext Context(TEXT(#Condition), __FILE__, __LINE__); \
			Context.WithHint(TEXT("Condition evaluated to true (expected false)")); \
			Context.ExecuteOrFail(); \
		} \
		FPalantirTrace::AddBreadcrumb(TEXT("AssertFalse"), TEXT(#Condition)); \
	} while(false)

