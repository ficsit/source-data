// Copyright 2016 Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FGCircuitConnectionComponent.h"
#include "FGWireHologram.generated.h"

#define NUM_CONNECTIONS 2

/**
 * Hologram for placing wires between circuit connections.
 */
UCLASS()
class FACTORYGAME_API AFGWireHologram : public AFGBuildableHologram
{
	GENERATED_BODY()
public:
	AFGWireHologram();

	// Begin AActor interface
	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty >& OutLifetimeProps ) const override;
	virtual void BeginPlay() override;
	virtual void Tick( float dt ) override;
	// End AActor interface

	// Begin AFGHologram Interface
	virtual AActor* Construct( TArray< AActor* >& out_children, FNetConstructionID netConstructionID ) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual bool TrySnapToActor( const FHitResult& hitResult ) override;
	virtual void SetHologramLocationAndRotation( const FHitResult& hitResult ) override;
	virtual void OnInvalidHitResult() override;
	virtual void SpawnChildren( AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator ) override;
	virtual void ScrollRotate( int32 delta, int32 step ) override;
	// End AFGHologram Interface

	// Begin AFGBuildableHologram Interface
	virtual void ConfigureActor( class AFGBuildable* inBuildable ) const override;
	// End AFGBuildableHologram Interface

	UFUNCTION(BlueprintPure, Category = "Power Pole")
	class AFGPowerPoleHologram* GetActiveAutomaticPoleHologram() const { return mActivePoleHologram; }

protected:
	UFUNCTION( BlueprintImplementableEvent, Category = "Hologram" )
	void OnAutomaticPoleDisableToggle( bool disabled );

	virtual void Destroyed() override;

	// Begin AFGHologram Interface
	virtual void CheckValidPlacement() override;
	virtual void SetupDepthMeshComponent( USceneComponent* attachParent, UMeshComponent* componentTemplate ) override;
	virtual void CheckClearance() override;
	// End AFGHologram Interface

private:
	void CheckValidSnap();
	void CheckLength();

	void SetActiveAutomaticPoleHologram( class AFGPowerPoleHologram* poleHologram );

	/**
	 * Check for nearby connectors.
	 * @return Closest connection, with or without free connections left.
	 */
	class UFGCircuitConnectionComponent* FindOverlappingCircuitConnectionComponent( const FVector& location, class AActor* actor, class UFGCircuitConnectionComponent* ignoredConnectionComponent );

	void StartLookAtBuilding( UFGCircuitConnectionComponent* overlappingComponent );
	void StopLookAtBuilding();

	void StartLookAtTooManyConnectionsBuilding( UFGCircuitConnectionComponent* overlappingConnection );

	float GetLength() const;

private:
	float mMaxLength;
	float mLengthPerCost;

	/** The two connection components we connect. */
	UPROPERTY()
	class UFGCircuitConnectionComponent* mConnections[ NUM_CONNECTIONS ];

	UPROPERTY( Replicated )
	class AFGPowerPoleHologram* mPowerPole;

	UPROPERTY( Replicated )
	class AFGPowerPoleWallHologram* mPowerPoleWall;

	class AFGPowerPoleHologram* mActivePoleHologram;

	UPROPERTY( EditDefaultsOnly, Category = "Power pole" )
	TSubclassOf<class UFGRecipe> mDefaultPowerPoleRecipe;

	UPROPERTY( EditDefaultsOnly, Category = "Power pole" )
	TSubclassOf<class UFGRecipe> mDefaultPowerPoleWallRecipe;

	/** Whether or not it's possible to place automatic power poles on the wall. */
	bool mAutomaticWallPoleEnabled;

	/** The start location of this wire */
	UPROPERTY( Replicated )
	FVector mStartLocation;

	/** Keeps track of what connection we are working with */
	UPROPERTY()
	int32 mCurrentConnection;

	/** The mesh we should stretch */
	UPROPERTY()
	UStaticMeshComponent* mWireMesh;
};
