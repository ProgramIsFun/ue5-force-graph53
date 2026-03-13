#pragma once

#include "CoreMinimal.h"

void ClearLogFile();
void LogAlways(const FString& TextToWrite);
void LogMessageInternal(const FString& StringToLog, bool LOG=false, int SeverityLevel = 0, const FString& Prefix = TEXT("[Info]"));

FVector Jiggle(const FVector& Vec, float Magnitude);
void eeeee();

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



    