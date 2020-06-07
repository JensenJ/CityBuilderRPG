// Copyright SpaceRPG 2020


#include "EnvironmentController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetTextLibrary.h"
#include "Math/Color.h"

// Sets default values
AEnvironmentController::AEnvironmentController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnvironmentController::BeginPlay()
{	
	Super::BeginPlay();

	//Limits size of array to 3 to stop overflow
	gameTime.SetNum(3);
	gameDate.SetNum(3);
	gameTemp.SetNum(3);

	lastWeather.SetNum(24);

	//Convert int to float for three main time variables
	hours = UKismetMathLibrary::Conv_IntToFloat(hours);
	minutes = UKismetMathLibrary::Conv_IntToFloat(minutes) / 60;
	seconds = UKismetMathLibrary::Conv_IntToFloat(seconds) / 3600;

	//Set starting time
	float startingTime = hours + minutes + seconds;
	clockwork = startingTime;

	//Sets game date varfiables
	gameDate.Insert(day, 0);
	gameDate.Insert(month, 1);
	gameDate.Insert(year, 2);
}

// Called every frame
void AEnvironmentController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetClockwork(DeltaTime);
	Clock();
	Calendar();
	EnvironmentTick(); //Updates all environment based stuff

	//Clamp arrays to 3 in size otherwise array grows rapidly
	gameTime.SetNum(3);
	gameDate.SetNum(3);
	gameTemp.SetNum(3);
}

//Ticks through and updates functions related to environment
void AEnvironmentController::EnvironmentTick() {
	seasonEnum = CalculateSeason(month);
	sunAngle = SetDayNight();
	tempFloat = CalculateTemperature();
	/*if (bIsTempFahrenheit) {
		TempString = FloatToDisplay(TempFloat, ESuffixEnum::EFahrenheit, true, 1);
	}
	else {
		TempString = FloatToDisplay(TempFloat, ESuffixEnum::ECelsius, true, 1);
	}*/

	weatherEnum = CalculateWeather();

	sunIntensity = CalculateSunIntensity();
	cloudSpeed = 100.0f;
	cloudOpacity = CalculateCloudOpacity();
	starOpacity = CalculateStarOpacity();

	CalculateSkyboxColour();
	UpdateEnvironment(); //Blueprint Function
}

//Sets clockwork for working out game speed.
void AEnvironmentController::SetClockwork(float DeltaSeconds) {
	//Works out game speed
	float DeltaTimeUnit = (DeltaSeconds / timeUnit * 0.24) * gameSpeedMultiplier;
	float AddedClockwork = DeltaTimeUnit + clockwork;
	float NewDayTick = AddedClockwork / (60 * 24);
	float Remainder = FGenericPlatformMath::Fmod(AddedClockwork, (60.0f * 24.0f));
	clockwork = Remainder;
	dayTick = NewDayTick;
}

//Calculates time
void AEnvironmentController::Clock() {
	// Seconds
	float clockworkSeconds = clockwork * 3600;
	float newSeconds = clockworkSeconds / 60;
	float RemainderSeconds = FGenericPlatformMath::Fmod(newSeconds, 60.0f);
	UKismetMathLibrary::FFloor(RemainderSeconds);
	seconds = RemainderSeconds;

	// Minutes
	float newMinutes = newSeconds / 60;
	float remainderMinutes = FGenericPlatformMath::Fmod(newMinutes, 60.0f);
	UKismetMathLibrary::FFloor(remainderMinutes);
	minutes = remainderMinutes;

	// Hours
	float NewHours = newMinutes / 60;
	dayNightHours = FGenericPlatformMath::Fmod(NewHours, 60.0f);
	UKismetMathLibrary::FFloor(dayNightHours);
	hours = dayNightHours;

	//Sets game time variables into array
	gameTime.Insert(seconds, 0);
	gameTime.Insert(minutes, 1);
	gameTime.Insert(hours, 2);

	//Logs time and whether day or night
	FString strHours = FString::FromInt(gameTime[2]);
	FString HoursMinutesString = UKismetStringLibrary::BuildString_Int(strHours, ":", gameTime[1], "");
	FString FinalString = UKismetStringLibrary::BuildString_Int(HoursMinutesString, ":", gameTime[0], "");
	UE_LOG(LogTemp, Warning, TEXT("EnvironmentController: Time: %s"), *FinalString);
	//UE_LOG(LogTemp, Warning, TEXT("Night: %s"), (bIsNight ? TEXT("True") : TEXT("False")));
}

//Calculates date
void AEnvironmentController::Calendar() {
	day = dayTick + day;
	int32 DaysInMonth = UKismetMathLibrary::DaysInMonth(year, month);
	if (day > DaysInMonth) {
		day = 1;
		month++;
	}
	if (month > 12) {
		month = 1;
		year++;
	}

	//Sets Game Date variables into array
	gameDate.Insert(day, 0);
	gameDate.Insert(month, 1);
	gameDate.Insert(year, 2);

	//Logs date
	FString strDays = FString::FromInt(gameDate[0]);
	FString DaysMonthString = UKismetStringLibrary::BuildString_Int(strDays, "/", gameDate[1], "");
	FString FinalString = UKismetStringLibrary::BuildString_Int(DaysMonthString, "/", gameDate[2], "");
	UE_LOG(LogTemp, Warning, TEXT("EnvironmentController: Date: %s"), *FinalString);
}

//Returns the season based on what month it is
ESeasonEnum AEnvironmentController::CalculateSeason(int32 m_month) {
	if (m_month == 12 || m_month == 1 || m_month == 2) {
		return ESeasonEnum::EWinter;
	}
	else if (m_month == 3 || m_month == 4 || m_month == 5) {
		return ESeasonEnum::ESpring;
	}
	else if (m_month == 6 || m_month == 7 || m_month == 8) {
		return ESeasonEnum::ESummer;
	}
	else if (m_month == 9 || m_month == 10 || m_month == 11) {
		return ESeasonEnum::EAutumn;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("EnvironmentController::Setting season failed!"));
		return ESeasonEnum::ENone;
	}
}

//Calculates SunAngle and returns to environment tick
FRotator AEnvironmentController::SetDayNight() {
	float m_sunAngle = ((dayNightHours / 6) * 90) + 90;
	FRotator sunRot = UKismetMathLibrary::MakeRotator(180, m_sunAngle, 180 + sunRotationOffset);

	return sunRot;
}

//Generates temperature for this hour.
float AEnvironmentController::CalculateTemperature() {
	if (bNewGenerationTemp) {
		lastTemp = FMath::RandRange(0.0f, 7.0f);
		bNewGenerationTemp = false;
	}

	if (gameTime[1] == 0) { //Resets temperature every hour when minute is 0 (new hour)
		if (!bHasGeneratedTemp) {
			for (int i = 0; i < 3; i++) { //Iterations for getting an average

				//Sets low and high bounds for each season
				if (seasonEnum == ESeasonEnum::EWinter) {
					maxGenTemp = 8.0f * tempMultiplier;
					minGenTemp = -7.0f * tempMultiplier;
				}
				else if (seasonEnum == ESeasonEnum::ESpring) {
					maxGenTemp = 12.0f * tempMultiplier;
					minGenTemp = 2.0f * tempMultiplier;
				}
				else if (seasonEnum == ESeasonEnum::ESummer) {
					maxGenTemp = 20.0f * tempMultiplier;
					minGenTemp = 7.0f * tempMultiplier;
				}
				else if (seasonEnum == ESeasonEnum::EAutumn) {
					maxGenTemp = 13.0f * tempMultiplier;
					minGenTemp = -3.0f * tempMultiplier;
				}
				else {
					//Error logging
					UE_LOG(LogTemp, Error, TEXT("Temperature::Check for season failed!"));
				}

				//Generate base temperature
				generatedTemp = FMath::RandRange(minGenTemp, maxGenTemp);

				//Makes sure temperature between last and current is not too far apart.
				if ((generatedTemp - lastTemp) > 4) {
					generatedTemp = lastTemp + FMath::RandRange(2.0f, 3.5f);
				}
				else if ((lastTemp - generatedTemp) > 4) {
					generatedTemp = lastTemp - FMath::RandRange(0.0f, 2.5f);
				}

				//Gradual increase towards midday
				if (gameTime[2] <= 13 && gameTime[2] > 1) {
					generatedTemp = generatedTemp + FMath::RandRange(1.5f, 3.0f);
				}
				//Gradual decrease towards midnight
				else if (gameTime[2] > 13 && gameTime[2] < 24) {
					generatedTemp = generatedTemp - FMath::RandRange(0.2f, 1.5f);
				}
				gameTemp.Insert(generatedTemp, i);
			}

			//Calculating Mean Temperature
			averageTemp = (gameTemp[0] + gameTemp[1] + gameTemp[2]) / 3;

			//averageTemp = AverageTemp - WindFloat / 5;

			if (averageTemp < -273.0f) {
				averageTemp = -273.0f;
			}
			lastTemp = averageTemp; //Setting last temp for next hour.
			//Converting celsius to fahrenheit if user has option enabled.
			/*if (bIsTempFahrenheit) {
				AverageTemp = (AverageTemp * (9 / 5)) + 32;
			}*/

			bHasGeneratedTemp = true; //Makes sure generation only happens once
		}
	}
	else {
		bHasGeneratedTemp = false; //Resets the variable for the next hour
	}

	return averageTemp; //Returns generated temp
}


//Function to calculate the sun intensity
float AEnvironmentController::CalculateSunIntensity() {
	float brightness = 10.0f;
	if (seasonEnum == ESeasonEnum::ESpring) {
		brightness *= 0.8;
	}
	else if (seasonEnum == ESeasonEnum::ESummer) {
		brightness *= 1.2;
	}
	else if (seasonEnum == ESeasonEnum::EAutumn) {
		brightness *= 0.7;
	}
	else if (seasonEnum == ESeasonEnum::EWinter) {
		brightness *= 0.5;
	}
	return brightness;
}

//Function to calculate the cloud opacity
float AEnvironmentController::CalculateCloudOpacity() {
	float newValue = 1;
	float value = 1;
	if (weatherEnum == EWeatherEnum::ESunny) {
		newValue = 0.8;
	}
	else if (weatherEnum == EWeatherEnum::EOvercast) {
		newValue = 3.0;
	}
	else if (weatherEnum == EWeatherEnum::ESnow) {
		newValue = 2.0;
	}
	else if (weatherEnum == EWeatherEnum::ERain) {
		newValue = 3.5;
	}
	else if (weatherEnum == EWeatherEnum::EThunder) {
		newValue = 4.0;
	}
	else {
		newValue = 1.0;
	}

	value = FMath::Lerp(cloudOpacity, newValue, gameSpeedMultiplier / (60 * 24));
	return value;
}

//Function to calculate the star opacity
float AEnvironmentController::CalculateStarOpacity() {
	float newValue = 1;
	float value = 1;
	if (weatherEnum == EWeatherEnum::ESunny) {
		newValue = 1.0;
	}
	else if (weatherEnum == EWeatherEnum::EOvercast) {
		newValue = 0.4;
	}
	else if (weatherEnum == EWeatherEnum::ESnow) {
		newValue = 0.2;
	}
	else  if (weatherEnum == EWeatherEnum::ERain) {
		newValue = 0.2;
	}
	else if (weatherEnum == EWeatherEnum::EThunder) {
		newValue = 0.0;
	}
	else {
		newValue = 1.0;
	}
	value = FMath::Lerp(starOpacity, newValue, gameSpeedMultiplier / (60 * 24));
	return value;
}

//Function to calculate the skybox colour based on the weather
void AEnvironmentController::CalculateSkyboxColour() {

	FLinearColor localZenith, localHorizon, localCloud, localOverall;

	if (weatherEnum == EWeatherEnum::ESunny) {
		localZenith = FLinearColor(0.1f, 0.2f, 0.3f, 1.0f);
		localHorizon = FLinearColor(0.45f, 0.5f, 0.75f, 1.0f);
		localCloud = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		localOverall = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (weatherEnum == EWeatherEnum::EOvercast) {
		localZenith = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
		localHorizon = FLinearColor(0.1f, 0.3f, 0.4f, 1.0f);
		localCloud = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
		localOverall = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
	}
	else if (weatherEnum == EWeatherEnum::ESnow) {
		localZenith = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
		localHorizon = FLinearColor(0.1f, 0.3f, 0.4f, 1.0f);
		localCloud = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
		localOverall = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
	}
	else if (weatherEnum == EWeatherEnum::ERain) {
		localZenith = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
		localHorizon = FLinearColor(0.0f, 0.2f, 0.3f, 1.0f);
		localCloud = FLinearColor(0.04f, 0.04f, 0.04f, 1.0f);
		localOverall = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}
	else if (weatherEnum == EWeatherEnum::EThunder) {
		localZenith = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
		localHorizon = FLinearColor(0.0f, 0.2f, 0.3f, 1.0f);
		localCloud = FLinearColor(0.04f, 0.04f, 0.04f, 1.0f);
		localOverall = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}
	else {
		localZenith = FLinearColor(0.1f, 0.2f, 0.3f, 1.0f);
		localHorizon = FLinearColor(0.0f, 0.6f, 1.0f, 1.0f);
		localCloud = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		localOverall = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//if sunrise/sunset
	if (gameTime[2] == 4 || gameTime[2] == 5 || gameTime[2] == 6 ||
		gameTime[2] == 16 || gameTime[2] == 17 || gameTime[2] == 18) {
		localZenith = FLinearColor(0.6f, 0.1f, 0.0f, 1.0f);
		localHorizon = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);
		localCloud = FLinearColor(0.3f, 0.2f, 0.0f, 1.0f);
	}
	//if night
	if ((gameTime[2] >= 19 && gameTime[2] <= 23) || (gameTime[2] >= 0 && gameTime[2] <= 3)) {
		localZenith = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		localHorizon = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
		localCloud = FLinearColor(0.04f, 0.04f, 0.04f, 1.0f);
		localOverall = FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}
	zenithColor = FMath::Lerp(zenithColor, localZenith, gameSpeedMultiplier / (60 * 24));
	horizonColor = FMath::Lerp(horizonColor, localHorizon, gameSpeedMultiplier / (60 * 24));
	cloudColor = FMath::Lerp(cloudColor, localCloud, gameSpeedMultiplier / (60 * 24));
	overallColor = FMath::Lerp(overallColor, localOverall, gameSpeedMultiplier / (60 * 24));
}

EWeatherEnum AEnvironmentController::CalculateWeather() {
	return EWeatherEnum::ESunny;
}