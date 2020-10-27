// Copyright 2016 Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FGBuildable.h"
#include "FGBuildableFactoryBuilding.generated.h"

UENUM()
enum class EFoundationSide : uint8
{
	FoundationFront UMETA(DisplayName = "Front"),
	FoundationRight UMETA( DisplayName = "Right" ),
	FoundationBack UMETA( DisplayName = "Back" ),
	FoundationLeft UMETA( DisplayName = "Left" ),
	FoundationTop UMETA( DisplayName = "Top" ),
	FoundationBottom UMETA( DisplayName = "Bottom" ),

	FoundationNumSides UMETA( DisplayName = "Num Sides" )
};

static FVector GetLocalSpaceNormalFromFoundationSide(EFoundationSide Side)
{
	switch( Side )
	{
		case EFoundationSide::FoundationFront:
			return FVector::ForwardVector;

		case EFoundationSide::FoundationRight:
			return FVector::RightVector;
		
		case EFoundationSide::FoundationBack:
			return FVector::BackwardVector;
		
		case EFoundationSide::FoundationLeft:
			return FVector::LeftVector;
			
		case EFoundationSide::FoundationTop:
			return FVector::UpVector;
		
		case EFoundationSide::FoundationBottom:
			return FVector::DownVector;
		
		default:
			// Is our game 4D now, wtf?
			break;
	}

	return FVector::ZeroVector;
}

/** Disable snapping on specific sides. */
USTRUCT( BlueprintType )
struct FFoundationSideSelectionFlags
{
	GENERATED_BODY()
public:
	static const FFoundationSideSelectionFlags NoEdges;
	static const FFoundationSideSelectionFlags AllEdges;

public:
	FFoundationSideSelectionFlags();
	FFoundationSideSelectionFlags( bool defaults );

	bool GetValueForSide( EFoundationSide Side );

	FFoundationSideSelectionFlags RotateEdges( int32 steps ) const;

public:
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Front : 1;
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Right : 1;
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Back : 1;
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Left : 1;
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Top : 1;
	UPROPERTY( EditDefaultsOnly, Category = "Sides" )
	uint8 Bottom : 1;
};

/**
 * Base for all kinds of factory building parts, like foundations, walls etc.
 */
UCLASS(Abstract)
class FACTORYGAME_API AFGBuildableFactoryBuilding : public AFGBuildable
{
	GENERATED_BODY()
public:
	AFGBuildableFactoryBuilding();

protected:
	FORCEINLINE class UFGColoredInstanceMeshProxy* GetMeshComponentProxy() const { return mMeshComponentProxy; }

private:
	/** Mesh component for the factory building. */
	UPROPERTY( VisibleAnywhere )
	class UFGColoredInstanceMeshProxy* mMeshComponentProxy = nullptr;
};
