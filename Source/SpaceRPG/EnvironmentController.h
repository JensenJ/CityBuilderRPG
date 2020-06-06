// Copyright SpaceRPG 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentController.generated.h"

UENUM(BlueprintType)
enum class ESeasonEnum : uint8 {
	ENone		UMETA(DisplayName = "None"),
	ESpring		UMETA(DisplayName = "Spring"),
	ESummer		UMETA(DisplayName = "Summer"),
	EAutumn		UMETA(DisplayName = "Autumn"),
	EWinter		UMETA(DisplayName = "Winter")
};

UENUM(BlueprintType)
enum class EWeatherEnum : uint8 {
	ENone		UMETA(DisplayName = "None"),
	ESunny		UMETA(DisplayName = "Sunny"),
	EOvercast	UMETA(DisplayName = "Overcast"),
	ERain		UMETA(DisplayName = "Rain"),
	ESnow		UMETA(DisplayName = "Snow"),
	EThunder	UMETA(DisplayName = "Thunder"),
};

UCLASS()
class SPACERPG_API AEnvironmentController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnvironmentController();

	//Arrays for time data and date data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	TArray<int32> gameTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	TArray<int32> gameDate;

	//Update function for environment
	UFUNCTION(BlueprintImplementableEvent, Category = "Calendar")
	void UpdateEnvironment();

	//Currently active season
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	ESeasonEnum seasonEnum;

	//Currently active weather
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	EWeatherEnum weatherEnum;

	//Temperature variable for calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float tempFloat;

	//Celestial / skysphere variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FRotator sunAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float sunIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float cloudSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float cloudOpacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float starOpacity; 

	//Skybox Colours for different weathers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skybox")
	FLinearColor zenithColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skybox")
	FLinearColor horizonColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skybox")
	FLinearColor cloudColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skybox")
	FLinearColor overallColor;

	//Multipliers.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calendar")
	float gameSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float tempMultiplier = 1.0f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:	
	//Clock functions
	void SetClockwork(float deltaSeconds);
	void Clock();
	void Calendar();

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

	//Environment functions
	void EnvironmentTick();
	ESeasonEnum CalculateSeason(int32 month);
	FRotator SetDayNight();
	float CalculateTemperature();
	float CalculateSunIntensity();
	float CalculateCloudOpacity();
	float CalculateStarOpacity();
	EWeatherEnum CalculateWeather();
	void CalculateSkyboxColour();

	//Temperature values
	TArray<float> gameTemp;
	float generatedTemp = 5.0f;
	float lastTemp;
	float minGenTemp;
	float maxGenTemp;
	float averageTemp;
	bool bHasGeneratedTemp = false;
	bool bNewGenerationTemp = true;

	//DayNight
	float dayNightHours = 0;

	//Weather values
	TArray<EWeatherEnum> lastWeather;
	EWeatherEnum weather;
	int32 weatherInt;
	int32 counter = 0;
	bool bHasGeneratedWeather = false;
	bool bNewGenerationWeather = true;
};
