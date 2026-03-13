#pragma once

#include "CoreMinimal.h"

void ClearLogFile();
void LogAlways(const FString& TextToWrite);
void LogMessageInternal(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"));

// Adds small random perturbation to near-zero vector components to prevent division by zero
FVector Jiggle(const FVector& Vec, float Magnitude);

// Debug breakpoint helper for critical errors during development
void GraphSystemDebugBreak();

inline FVector RandPointInCircle(float CircleRadius)
{
	FVector Point;
	FVector::FReal L;

	do
	{
		// Check random vectors in the unit circle so result is statistically uniform.
		Point.X = FMath::FRand() * 2.f - 1.f;
		Point.Y = FMath::FRand() * 2.f - 1.f;
		Point.Z = FMath::FRand() * 2.f - 1.f;
		L = Point.SizeSquared();
	}
	while (L > 1.0f);

	return Point * CircleRadius;
}



    