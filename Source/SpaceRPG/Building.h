// Copyright SpaceRPG 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

UCLASS()
class SPACERPG_API ABuilding : public AActor
{
	GENERATED_BODY()
	
	//Static mesh to use for the building
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Building, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildingMesh;


public:	
	// Sets default values for this actor's properties
	ABuilding();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	TArray<FVector> snapPositions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building, meta = (AllowPrivateAccess = "true"))
	FVector buildingBounds;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
