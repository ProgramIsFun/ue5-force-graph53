// GraphPhysicsSimulator.h
// Handles all physics simulation: force calculations, velocity updates, alpha decay

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GraphConfiguration.h"
#include "GraphPhysicsSimulator.generated.h"

// Forward declarations
struct OctreeNode;
class Node77;
class Link77;

/**
 * Physics simulation parameters
 */
USTRUCT(BlueprintType)
struct FPhysicsParameters
{
	GENERATED_BODY()

	// Alpha (simulation temperature)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float Alpha = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaMin = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AlphaDecay = 0.0228f; // 1 - pow(0.001, 1.0/300)

	// Force parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float EdgeDistance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float NodeStrength = -60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float VelocityDecay = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DistanceMin = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float DistanceMax = 10000000.0f;

	// Initial positioning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float InitialRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float UniversalGraphScale = 1.0f;
};

/**
 * Manages all physics simulation for the force-directed graph
 */
UCLASS()
class NBODYSIMULATION_API UGraphPhysicsSimulator : public UObject
{
	GENERATED_BODY()

public:
	// Initialize with configuration
	void Initialize(const FGraphConfiguration& Config, const FPhysicsParameters& Params);

	// Main simulation step
	void SimulateStep(
		float DeltaTime,
		TArray<FVector>& NodePositions,
		TArray<FVector>& NodeVelocities,
		const TArray<Node77>& Nodes,
		const TArray<Link77>& Links
	);

	// Check if simulation has stabilized
	bool IsStabilized() const;

	// Get current alpha value
	float GetAlpha() const { return PhysicsParams.Alpha; }

	// Set alpha (useful for restarting simulation)
	void SetAlpha(float NewAlpha) { PhysicsParams.Alpha = NewAlpha; }

	// Get iteration count
	int32 GetIterations() const { return Iterations; }

private:
	// Configuration
	FGraphConfiguration Config;
	FPhysicsParameters PhysicsParams;
	int32 Iterations = 0;

	// Force calculation methods
	void CalculateLinkForces(
		TArray<FVector>& NodeVelocities,
		const TArray<FVector>& NodePositions,
		const TArray<Link77>& Links
	);

	void CalculateChargeForces(
		TArray<FVector>& NodeVelocities,
		const TArray<FVector>& NodePositions,
		const TArray<Node77>& Nodes
	);

	void CalculateChargeForcesOctree(
		TArray<FVector>& NodeVelocities,
		const TArray<FVector>& NodePositions,
		const TArray<Node77>& Nodes
	);

	void CalculateChargeForcesBruteForce(
		TArray<FVector>& NodeVelocities,
		const TArray<FVector>& NodePositions,
		const TArray<Node77>& Nodes
	);

	void CalculateCenterForce(
		TArray<FVector>& NodePositions,
		const FVector& Center
	);

	// Velocity and position updates
	void UpdateVelocitiesWithDecay(TArray<FVector>& NodeVelocities);
	
	void UpdatePositionsFromVelocities(
		TArray<FVector>& NodePositions,
		const TArray<FVector>& NodeVelocities
	);

	// Alpha management
	void UpdateAlpha();

	// Helper for jiggle (small random perturbation)
	FVector Jiggle(const FVector& V, float Magnitude) const;

	// Logging helper
	void LogMessage(const FString& Message, bool bForceLog = false) const;
};
