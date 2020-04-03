// Copyright 2016-2019 Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "FGInstancedSplineMeshComponent.h"
#include "FGSplineMeshGenerationLibrary.generated.h"

/**
 * Helper library for generating spline meshes, collision and meshes along a splines.
 */
UCLASS()
class FACTORYGAME_API UFGSplineMeshGenerationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * Given a spline, this creates non-instanced spline meshes along the spline.
	 *
	 * @param spline			The spline the meshes should go along.
	 *
	 * @param mesh				The static mesh to spline, it will be splined along the X-axis.
	 *
	 * @param meshLength		The length of the given mesh, along it's X-axis.
	 *
	 * @param meshPool			Mesh pool to use for the created spline meshes, new meshes are created
	 *							as needed and old meshes are removed if there are too many in a reused pool.
	 *							Pools can be reused and the meshes will be re-splined to fix the current need,
	 *							This is useful for holograms as the spline change around a lot.
	 *
	 * @param meshConstructor	Lambda or other function that creates a new mesh to put in the pool.
	 *							The constructor is responsible for calling SetupAttachment and set the Owner.
	 *							The constructor must never call RegisterComponent.
	 *							If the returned mesh have mobility Static, then the pool cannot be reused as any further changes to the mesh have no effect.
	 *
	 *							Example:
	 *							[]( USplineComponent* spline )
	 *							{
	 *								USplineMeshComponent* newComponent = NewObject< USplineMeshComponent >( spline->GetOwner(), USplineMeshComponent::StaticClass() );
	 *
	 *								newComponent->SetupAttachment( spline );
	 *								newComponent->Mobility = EComponentMobility::Movable;
	 *
	 *								// Custom initialization steps
	 *
	 *								return newComponent;
	 *							}
	 */
	template< typename MeshConstructor >
	static void BuildSplineMeshes(
		class USplineComponent* spline,
		UStaticMesh* mesh,
		float meshLength,
		TArray< USplineMeshComponent* >& meshPool,
		MeshConstructor meshConstructor );

	template< typename MeshConstructor >
	static void BuildSplineMeshes(
		class USplineComponent* spline,
		UStaticMesh* mesh,
		float meshLength,
		int32 maxNumMeshes,
		TArray< USplineMeshComponent* >& meshPool,
		MeshConstructor meshConstructor );

	/**
	 * Given a spline, this creates an instanced spline mesh along the spline.
	 *
	 * @param spline, mesh, meshLength		See BuildSplineMeshes above.
	 *
	 * @param splineInstances				The instance component to fill up with spline instances.
	 *										This can be reused between calls to update an existing one.
	 *										If this have mobility Static, it must not be registered before calling this function, if it is then this function have no effect.
	 */
	static void BuildSplineMeshesInstanced(
		class USplineComponent* spline,
		UStaticMesh* mesh,
		float meshLength,
		UFGInstancedSplineMeshComponent* splineInstances );

	/**
	 * Given a spline, this creates collisions along the spline.
	 *
	 * @param spline			The spline the collisions should go along.
	 * @param collisionExtent	The size of the collision, X is forward along the spline, Y is sideways and Z is height.
	 * @param collisionSpacing	The distance between each collision.
	 * @param collisionOffset	The offset of the collision in the spline local space, see collisionExtent above.
	 * @param collisionProfile	The collision profile to apply for the generated collision.
	 *
	 * @note The created collisions are registered and attached to the spline with the same owning actor.
	 */
	static void BuildSplineCollisionBoxes(
		class USplineComponent* spline,
		const FVector& collisionExtent,
		float collisionSpacing,
		const FVector& collisionOffset,
		FName collisionProfile );


	/**
	 * Given a spline, this creates collisions along the spline.
	 *
	 * @param spline			The spline the collisions should go along.
	 * @param collisionRadius	The radius of the collision on the spline.
	 * @param collisionSpacing	The distance between each collision.
	 * @param collisionOffset	The offset of the collision in the spline local space, see collisionExtent above.
	 * @param collisionProfile	The collision profile to apply for the generated collision.
	 *
	 * @note The created collisions are registered and attached to the spline with the same owning actor.
	 */
	static void BuildSplineCollisionCapsules( USplineComponent* spline, float collisionRadius, float collisionSpacing, const FVector& collisionOffset, FName collisionProfile );

	/** GetNextDistanceExcedingTollerance used to step though a spline to take as long straight steps as possible within an error threshold
	 *
	 * @param	startPos - position on spline. In sync with startDistance. Only sent in so we needing less fetches from the spline, as it can be used from last result in a loop
	 * @param	startDistance - distance on spline to start from
	 * @param	stepSize - the distance to take in each test. (will use half distance to search back when violation of limits exceeded)
	 * @param	tollerance - see it as a bound value that as long as the curve stay within, we continue
	 * @param	outEndDistance - the end distance that is just within the threshold
	 * @param	outEndPos - the position on the spline at the end distance
	 * @param	outLength - the length of the segment connecting the start and the end point. Can be used to calculate a direction of the segment if needed for collision or similar
	 * @param	fineTuningIterations - once found the step that takes us outside, how many times should that be refined with binary search to get closer to the exact position? See this a a precision thing.
	 * @param	minStepFactor - what factor of the step size to use for the minimum step we can accept (used to make sure we don't take a too small first step and that we don't leave too little behind in the end. Min step pwill be this factor * stepSize
	 *
	 * @return	bool - returns false if we've reached the end and true if there is still more left
	 */
	static bool GetNextDistanceExcedingTollerance( USplineComponent* spline, const FVector& startPos, float startDistance, float stepSize, float tollerance, float& outEndDistance, FVector& outEndPos, float& outLength, uint8 fineTuningIterations = 5, float minStepFactor = 0.5f, ESplineCoordinateSpace::Type space = ESplineCoordinateSpace::World );
};


/**
 * Templated function implementations.
 */
template< typename MeshConstructor >
void UFGSplineMeshGenerationLibrary::BuildSplineMeshes(
	class USplineComponent* spline,
	UStaticMesh* mesh,
	float meshLength,
	TArray< USplineMeshComponent* >& meshPool,
	MeshConstructor meshConstructor )
{
	check( spline );

	const float splineLength = spline->GetSplineLength();
	const int32 numMeshes = FMath::Max( 1, FMath::RoundToInt( splineLength / meshLength ) ); //[DavalliusA:Tue/25-02-2020] removed a +1 here that as messing with conveyors max length (me and G2 could not see why we needed the +1 )

	// Create more or remove the excess meshes.
	if( numMeshes < meshPool.Num() )
	{
		while( meshPool.Num() > numMeshes )
		{
			meshPool.Last()->DestroyComponent();
			meshPool.Pop();
		}
	}
	else if( numMeshes > meshPool.Num() )
	{
		while( meshPool.Num() < numMeshes )
		{
			auto newMesh = meshConstructor( spline );
			if( newMesh ) //[DavalliusA:Wed/29-01-2020] added null handling. No reason not to do it really. Only question is if we should break or not? But as it's a new thing, let's do the cheapest alternative for now and break. Feel free to change it later.
			{

				meshPool.Push( newMesh );
			}
			else
			{
				break;
			}
		}
	}

	// Put all pieces along the spline.
	for( int32 i = 0; i < meshPool.Num(); ++i )
	{
		const float segmentLength = splineLength / numMeshes;
		const float startDistance = (float )i * segmentLength;
		const float endDistance = (float )( i + 1 ) * segmentLength;
		const FVector startPos = spline->GetLocationAtDistanceAlongSpline( startDistance, ESplineCoordinateSpace::Local );
		const FVector startTangent = spline->GetTangentAtDistanceAlongSpline( startDistance, ESplineCoordinateSpace::Local ).GetSafeNormal() * segmentLength;
		const FVector endPos = spline->GetLocationAtDistanceAlongSpline( endDistance, ESplineCoordinateSpace::Local );
		const FVector endTangent = spline->GetTangentAtDistanceAlongSpline( endDistance, ESplineCoordinateSpace::Local ).GetSafeNormal() * segmentLength;

		meshPool[ i ]->SetStartAndEnd( startPos, startTangent, endPos, endTangent, true );
		meshPool[ i ]->SetStaticMesh( mesh );
	}

	// Register new meshes, needs to happen after the properties are set for static components.
	for( auto meshComp : meshPool )
	{
		if( !meshComp->IsRegistered() )
		{
			meshComp->RegisterComponent();
		}
	}
}

template< typename MeshConstructor >
void UFGSplineMeshGenerationLibrary::BuildSplineMeshes(
	class USplineComponent* spline,
	UStaticMesh* mesh,
	float meshLength,
	int32 maxNumMeshes,
	TArray< USplineMeshComponent* >& meshPool,
	MeshConstructor meshConstructor )
{
	check( spline );

	const float splineLength = spline->GetSplineLength();
	const int32 numMeshes = FMath::Max( 1, FMath::RoundToInt( splineLength / meshLength ) );  //[DavalliusA:Tue/25-02-2020] removed a +1 here that as messing with conveyors max length (me and G2 could not see why we needed the +1 )
	//[DavalliusA:Wed/29-01-2020] don't apply max value here, as that will make the meshes stretch over the full length of the spline still instead of cutting of early.

	// Create more or remove the excess meshes.
	if( numMeshes < meshPool.Num() )
	{
		while( meshPool.Num() > numMeshes )
		{
			meshPool.Last()->DestroyComponent();
			meshPool.Pop();
		}
	}
	else if( numMeshes > meshPool.Num() )
	{
		while( meshPool.Num() < numMeshes && meshPool.Num() < maxNumMeshes )
		{
			auto newMesh = meshConstructor( spline );
			if( newMesh ) //[DavalliusA:Wed/29-01-2020] added null handling. No reason not to do it really. Only question is if we should break or not? But as it's a new thing, let's do the cheapest alternative for now and break. Feel free to change it later.
			{

				meshPool.Push( newMesh );
			}
			else
			{
				break;
			}
		}
	}


	const float segmentLength = splineLength / numMeshes;
	// Put all pieces along the spline.
	for( int32 i = 0; i < meshPool.Num(); ++i )
	{
		const float startDistance = (float )i * segmentLength;
		const float endDistance = (float )( i + 1 ) * segmentLength;
		const FVector startPos = spline->GetLocationAtDistanceAlongSpline( startDistance, ESplineCoordinateSpace::Local );
		const FVector startTangent = spline->GetTangentAtDistanceAlongSpline( startDistance, ESplineCoordinateSpace::Local ).GetSafeNormal() * segmentLength;
		const FVector endPos = spline->GetLocationAtDistanceAlongSpline( endDistance, ESplineCoordinateSpace::Local );
		const FVector endTangent = spline->GetTangentAtDistanceAlongSpline( endDistance, ESplineCoordinateSpace::Local ).GetSafeNormal() * segmentLength;

		meshPool[ i ]->SetStartAndEnd( startPos, startTangent, endPos, endTangent, true );
		meshPool[ i ]->SetStaticMesh( mesh );
	}

	// Register new meshes, needs to happen after the properties are set for static components.
	for( auto meshComp : meshPool )
	{
		if( !meshComp->IsRegistered() )
		{
			meshComp->RegisterComponent();
		}
	}
}
