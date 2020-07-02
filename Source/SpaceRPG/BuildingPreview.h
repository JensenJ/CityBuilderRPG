// Copyright SpaceRPG 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingPreview.generated.h"

UCLASS()
class SPACERPG_API ABuildingPreview : public AActor
{
	GENERATED_BODY()
	

public:	
	// Sets default values for this actor's properties
	ABuildingPreview();

	//Components for preview setup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BuildPreviewSetup, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BuildPreviewSetup, meta = (AllowPrivateAcxess = "true"))
	class UBoxComponent* OverlapBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BuildPreviewSetup, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* validMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BuildPreviewSetup, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* invalidMaterial;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:	

	//Variables for build preview setup
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildPreviewSetup, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"))
	class ASpaceRPGCharacter* owningPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildPreviewSetup, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"))
	class UStaticMesh* buildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true"))
	class ABuilding* hitBuilding;

	//Variables for building mode
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"))
	bool bIsGridSnappingEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true", ExposeOnSpawn = "true"))
	bool bIsBuildSnappingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true"))
	float buildingRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true"))
	float rotationSnapAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = BuildSettings, meta = (AllowPrivateAccess = "true"));
	bool bIsPlacementValid = false;

	bool bHasOverlappingActors = false;

	float currentZRotationValue = 0.0f;


	//Functions to detect collisions
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* overlappedComponent, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* overlappedComponent, class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 OtherBodyIndex);

	//Functions to toggle snapping
	UFUNCTION(BlueprintCallable)
	void SetGridSnapping(bool value);

	UFUNCTION(BlueprintCallable)
	void SetBuildSnapping(bool value);

	//Functions to rotate the object
	UFUNCTION(BlueprintCallable)
	void RotateAntiClockwise();

	UFUNCTION(BlueprintCallable)
	void RotateClockwise();

	//Functions to set placement validity
	UFUNCTION(BlueprintCallable)
	void SetInvalidPlacement();

	UFUNCTION(BlueprintCallable)
	void SetValidPlacement();

	//Function to find the closest snap point in a building
	FVector FindClosestSnapPoint(FVector hitlocation, class ABuilding* hitBuilding);

	//Function to check the conditions for the placement validity
	void CheckBuildingConditions();

	//Function to run grid snap checks and apply actor location
	void GridSnapping(FVector location);
};