// Copyright SpaceRPG 2020


#include "BuildingPreview.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "SpaceRPGCharacter.h"
#include "Camera/CameraComponent.h"
#include "Building.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
ABuildingPreview::ABuildingPreview()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Setup components
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(StaticMesh);

	//Create default materials
	validMaterial = CreateDefaultSubobject<UMaterial>(TEXT("ValidMaterial"));
	invalidMaterial = CreateDefaultSubobject<UMaterial>(TEXT("InvalidMaterial"));
}

// Called when the game starts or when spawned
void ABuildingPreview::BeginPlay()
{
	Super::BeginPlay();
	
	//Null pointer checks
	if (buildingMesh == nullptr)
	{ 
		UE_LOG(LogTemp, Error, TEXT("BuildingPreview::Building mesh has not been set, make sure it is set when the object is spawned."))
		return; 
	}
	if (StaticMesh == nullptr)
	{ 
		UE_LOG(LogTemp, Error, TEXT("BuildingPreview::Static Mesh has not been initialised correctly."))
		return; 
	}
	if (invalidMaterial == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingPreview::Building Invalid material has not been set."))
		return;
	}
	if (owningPlayer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingPreview::The owning player has not been set. Make sure it is set when the object is spawned."))
	}

	//Set the mesh to use the building mesh
	StaticMesh->SetStaticMesh(buildingMesh);

	//Set the placement to be invalid by default
	SetInvalidPlacement();

	//Getting the bounds of the static mesh object
	FVector origin;
	FVector boxExtent;
	float sphereRadius;

	UKismetSystemLibrary::GetComponentBounds(StaticMesh, origin, boxExtent, sphereRadius);

	//Setting new positions and scale for the overlap box
	OverlapBox->SetBoxExtent(boxExtent * 0.9999f);
	OverlapBox->AddLocalOffset(FVector(0, 0, boxExtent.Z));

	//Bind box collider functions
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABuildingPreview::OnOverlapBegin);
	OverlapBox->OnComponentEndOverlap.AddDynamic(this, &ABuildingPreview::OnOverlapEnd);

	//Set new collision response so character can walk inside preview
	StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

//Collision Detection Functions

void ABuildingPreview::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Find any overlapping actors
	TArray<AActor*> overlappingActors;
	OverlapBox->GetOverlappingActors(overlappingActors, TSubclassOf<AActor>());

	//Check if the number of overlapping actors is greater than 0
	if (overlappingActors.Num() > 0) 
	{
		//If so, we have overlapping actors
		bHasOverlappingActors = true;
	}
}

void ABuildingPreview::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//Find any overlapping actors
	TArray<AActor*> overlappingActors;
	OverlapBox->GetOverlappingActors(overlappingActors, TSubclassOf<AActor>());

	//check if the number of overlapping actors is equal to 0
	if (overlappingActors.Num() == 0) 
	{
		//If so, we have no overlapping actors
		bHasOverlappingActors = false;
	}
}

//Toggle snap mode functions
void ABuildingPreview::SetGridSnapping(bool value)
{
	bIsGridSnappingEnabled = value;
}

void ABuildingPreview::SetBuildSnapping(bool value)
{
	bIsBuildSnappingEnabled = value;
}

//Rotation functions
void ABuildingPreview::RotateAntiClockwise()
{
	currentZRotationValue += rotationSnapAngle;
	SetActorRotation(FRotator(0.0f, currentZRotationValue, 0.0f));
}

void ABuildingPreview::RotateClockwise() 
{
	currentZRotationValue -= rotationSnapAngle;
	SetActorRotation(FRotator(0.0f, currentZRotationValue, 0.0f));
}

//Functions to set validity of placement
void ABuildingPreview::SetInvalidPlacement()
{
	//Get all materials on the static mesh
	TArray<UMaterialInterface*> materials = StaticMesh->GetMaterials();

	//Loop through the materials and set them to the invalid material
	for (int32 i = 0; i < materials.Num(); i++)
	{
		//Set Material instance
		StaticMesh->SetMaterial(i, invalidMaterial);
	}

	bIsPlacementValid = false;
}

void ABuildingPreview::SetValidPlacement()
{
	//Get all materials on the static mesh
	TArray<UMaterialInterface*> materials = StaticMesh->GetMaterials();

	//Loop through the materials and set them to the invalid material
	for (int32 i = 0; i < materials.Num(); i++)
	{
		//Set Material instance
		StaticMesh->SetMaterial(i, validMaterial);
	}
	
	bIsPlacementValid = true;
}

// Called every frame
void ABuildingPreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Null check
	if (owningPlayer == nullptr)
	{
		return;
	}

	//Getting vectors of the first person camera
	UCameraComponent* camera = owningPlayer->GetFirstPersonCamera();

	//Check if the camera returned was valid
	if (camera == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find first person camera."))
		return;
	}
	FVector forwardCamera = camera->GetForwardVector() * buildingRange;
	FVector cameraLocation = camera->GetComponentLocation();
	
	FVector cameraEndVector = forwardCamera + cameraLocation;

	//Line trace
	FHitResult lineHit;

	FCollisionQueryParams traceParams;
	bool lineTraceSuccess = GetWorld()->LineTraceSingleByChannel(lineHit, cameraLocation, cameraEndVector, ECC_Visibility, traceParams);

	//If the line trace hit something
	if (lineTraceSuccess == true)
	{
		//See if it hit a building
		hitBuilding = Cast<ABuilding>(lineHit.GetActor());

		FVector lineHitLocation = lineHit.Location;

		//If it was a building
		if (hitBuilding != nullptr)
		{
			//If build snapping is enabled
			if (bIsBuildSnappingEnabled == true)
			{
				//Find the cloest snap point
				FVector snapPoint = FindClosestSnapPoint(lineHitLocation, hitBuilding);

				//Set the actor's location 
				SetActorLocation(hitBuilding->GetActorLocation() + snapPoint);
			}
			else
			{
				GridSnapping(lineHitLocation);
			}
		}
		//If a building was not hit
		else
		{
			//Calculating Box Trace Start Value
			FVector boxExtent = OverlapBox->GetScaledBoxExtent();
			FVector boxTraceStart = FVector(lineHitLocation.X, lineHitLocation.Y, lineHitLocation.Z + boxExtent.Z);

			//Make the collision box the size of the overlap box
			FCollisionShape Shape = FCollisionShape::MakeBox(boxExtent);

			//The hit result
			FHitResult boxHit;

			//Do the box trace, looking for world dynamic objects (buildings)
			bool boxTraceSuccess = GetWorld()->SweepSingleByObjectType(boxHit, boxTraceStart, boxTraceStart + 0.1f, FQuat(), ECC_WorldDynamic, Shape);

			//If the box trace hit something
			if (boxTraceSuccess == true)
			{
				//See if the object we hit was a building
				hitBuilding = Cast<ABuilding>(boxHit.GetActor());

				FVector boxHitLocation = boxHit.Location;

				//If the object was a building and the build snapping is enabled
				if (hitBuilding != nullptr && bIsBuildSnappingEnabled)
				{
					//Find the closest building snap point
					FVector snapPoint = FindClosestSnapPoint(boxHitLocation, hitBuilding);
					SetActorLocation(snapPoint + hitBuilding->GetActorLocation());
				}
				else
				{
					GridSnapping(lineHitLocation);
				}
			}
			else
			{
				GridSnapping(lineHitLocation);
			}

		}

		//Check the building conditions
		CheckBuildingConditions();
	}
	//If nothing was hit by the line trace
	else
	{
		//If grid snapping is enabled
		if (bIsGridSnappingEnabled == true)
		{
			//Snap the location to the grid and set the actors location to it.
			FVector snappedLocation = UKismetMathLibrary::Vector_SnappedToGrid(cameraEndVector, 100.0f);
			SetActorLocation(snappedLocation);
		}
		else
		{
			//Set the location to the camera end vector
			SetActorLocation(cameraEndVector);
		}

		//Set placement invalid as buildings shouldn't be placeable in mid-air
		SetInvalidPlacement();
	}
}

//Function to check the validity of the building conditions
void ABuildingPreview::CheckBuildingConditions()
{
	//Check if this building is overlapping / colliding with anything
	if (!bHasOverlappingActors)
	{
		//Only set the materials if they need to be set
		if (bIsPlacementValid == false)
		{
			SetValidPlacement();
		}
	}
	else
	{
		//Only set the materials if they need to be set
		if (bIsPlacementValid == true)
		{
			SetInvalidPlacement();
		}
	}
}

//Function to run grid snapping
void ABuildingPreview::GridSnapping(FVector location)
{
	//If grid snapping is enabled
	if (bIsGridSnappingEnabled)
	{
		//Snap the location to the grid and set the actors location to it.
		FVector snappedLocation = UKismetMathLibrary::Vector_SnappedToGrid(location, 100.0f);
		SetActorLocation(snappedLocation);
	}
	//If grid snapping is disabled
	else
	{
		SetActorLocation(location);
	}
}

//Function to find the cloest snap point on the building
FVector ABuildingPreview::FindClosestSnapPoint(FVector hitLocation, class ABuilding* m_hitBuilding)
{
	if (m_hitBuilding == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BuildingPreview::Finding snap point failed, building was null"))
		return hitLocation;
	}

	TArray<FVector> SnapPositions = m_hitBuilding->snapPositions;
	FVector buildingLocation = m_hitBuilding->GetActorLocation();

	FVector snappingOffset;
	float closestDistance;

	//For every position in the building array
	for (int32 i = 0; i < SnapPositions.Num(); i++)
	{
		//Calculate the vector length for this snap point
		float vectorLength = (hitLocation - (SnapPositions[i] + buildingLocation)).Size();
		
		//If this is the first snap posiiton, set values
		if (i == 0)
		{
			closestDistance = vectorLength;
			snappingOffset = SnapPositions[i];
		}
		else
		{
			//Check if the new vector length is less than the closest distance
			if (vectorLength < closestDistance)
			{
				//Set values
				closestDistance = vectorLength;
				snappingOffset = SnapPositions[i];
			}
		}
	}

	//Return the shortest snapping offset
	return snappingOffset * 2.0f;
}