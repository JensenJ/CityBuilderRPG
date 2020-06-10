// Copyright SpaceRPG 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeController.generated.h"

UCLASS()
class SPACERPG_API ATimeController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimeController();

	//Networking clockwork
	UPROPERTY(ReplicatedUsing=OnRep_Clockwork)
	float net_clockwork;

	UFUNCTION()
	void OnRep_Clockwork();
	
	//Networking calendar
	UPROPERTY(ReplicatedUsing=OnRep_Calendar)
	TArray<int32> net_GameDate;

	UFUNCTION()
	void OnRep_Calendar();

	//Arrays for time data and date data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	TArray<int32> gameTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	TArray<int32> gameDate;

	//Update function for Time, runs every frame to update the directional light and the skysphere
	UFUNCTION(BlueprintImplementableEvent, Category = "Calendar")
	void UpdateTime();

	//Update function for every hour
	UFUNCTION(BlueprintImplementableEvent, Category = "Calendar")
	void UpdateHour();

	//Update function for every day
	UFUNCTION(BlueprintImplementableEvent, Category = "Calendar")
	void UpdateDay();

	//Celestial / skysphere variables
	//Sun Angle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	FRotator sunAngle;

	//Sun height, used for calculating intensity, set within blueprint updateTime function
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float sunHeight;

	//Value for the sun intensity, needs to be assigned to the directional light
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
	float sunIntensity;

	//Set in editor to offset the sun's rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float sunRotationOffset;

	//Multipliers.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	float gameSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float sunIntensityMultiplier = 1.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:	
	//Clock functions
	void SetClockwork(float deltaSeconds);
	void Clock();
	void Calendar();
	void OnHourChanged();
	void OnDayChanged();

	//Clock variables
	float timeUnit = 0.25f;
	float clockwork;
	float dayTick = 0;
	int32 seconds = 0;
	int32 minutes = 0;
	int32 hours = 8 * 60;
	int32 day = 1;
	int32 month = 1;
	int32 year = 1;

	int32 lastHour;
	int32 lastDay;

	//Time functions
	//Function for every tick to update Time related stuff
	void TimeTick();
	//Get the sun's rotation based on time
	FRotator SetDayNight();
	//Function to calculate the sun intensity based on the sun's height in the skysphere
	float CalculateSunIntensity();

	//DayNight
	float dayNightHours = 0;

};
