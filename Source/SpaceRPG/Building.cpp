// Copyright SpaceRPG 2020

#include "Building.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"

// Sets default values
ABuilding::ABuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	RootComponent = BuildingMesh;
}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	//Calculate Building Bounds
	//Creating vectors for outputs of GetActorBounds
	FVector boxExtent;
	FVector origin;

	AActor::GetActorBounds(false, origin, boxExtent, false);

	//Multiplying to make bounding box slightly smaller so buildings can be placed directly next to each other without collisions blocking it.
	boxExtent *= 0.99f;

	//Dividing by 100 for bounding box to be snapped to nearest 100
	boxExtent /= 100.0f;

	//Working out upper bounds of x, y, z for building bounds
	int32 x = UKismetMathLibrary::FCeil(boxExtent.X);
	int32 y = UKismetMathLibrary::FCeil(boxExtent.Y);
	int32 z = UKismetMathLibrary::FCeil(boxExtent.Z);

	//Calculating final buildings bound, snapped to the nearest 100
	buildingBounds = FVector(x, y, z) * 100.0f;

	//Filling snap positions array
	snapPositions.Insert(FVector(buildingBounds.X, 0, 0), 0);
	snapPositions.Insert(FVector(buildingBounds.X * -1.0f, 0, 0), 1);
	snapPositions.Insert(FVector(0, buildingBounds.Y, 0), 2);
	snapPositions.Insert(FVector(0, buildingBounds.Y * -1.0f, 0), 3);
	//snapPositions.Insert(FVector(0, 0, buildingBounds.Z), 4);
	//snapPositions.Insert(FVector(0, 0, buildingBounds.Z * -1.0f), 5);

	//Ensure size of snap positions is kept at 4
	snapPositions.SetNum(4);
}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

