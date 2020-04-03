// Copyright 2020 Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "FGReplicationDetailActor_GeneratorFuel.h"
#include "FGReplicationDetailActor_GeneratorNuclear.generated.h"

UCLASS()
class FACTORYGAME_API AFGReplicationDetailActor_GeneratorNuclear : public AFGReplicationDetailActor_GeneratorFuel
{
	GENERATED_BODY()
	
public:
	AFGReplicationDetailActor_GeneratorNuclear();

	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
	virtual void InitReplicationDetailActor( class AFGBuildable* owningActor ) override;
	virtual void FlushReplicationActorStateToOwner() override;
	virtual bool HasCompletedInitialReplication() const override;

	FORCEINLINE UFGInventoryComponent* GetWasteInventory() const { return mWasteInventory; }

protected:
	UPROPERTY( Replicated )
	class UFGInventoryComponent* mWasteInventory;
};