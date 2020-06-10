// Copyright SpaceRPG 2020

#include "TimeController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "GameFramework/Actor.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetTextLibrary.h"
#include "Math/Color.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ATimeController::ATimeController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
}

void ATimeController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATimeController, net_clockwork);
	DOREPLIFETIME(ATimeController, net_GameDate);
}

// Called when the game starts or when spawned
void ATimeController::BeginPlay()
{	
	Super::BeginPlay();

	//Limits size of array to 3 to stop overflow
	gameTime.SetNum(3);
	gameDate.SetNum(3);

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

	lastHour = hours;
	OnHourChanged();

	lastDay = day;
	OnDayChanged();
}

// Called every frame
void ATimeController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Time based stuff
	SetClockwork(DeltaTime);
	Clock();
	Calendar();
	TimeTick(); //Updates all Time based stuff

	//Clamp arrays to 3 in size otherwise array grows rapidly
	gameTime.SetNum(3);
	gameDate.SetNum(3);
}

//Ticks through and updates functions related to Time
void ATimeController::TimeTick() {
	sunAngle = SetDayNight(); //Calculate sun angle
	sunIntensity = CalculateSunIntensity();

	UpdateTime(); //Blueprint Function called
}

//Sets clockwork for working out game speed.
void ATimeController::SetClockwork(float DeltaSeconds) {
	//Works out game speed
	float DeltaTimeUnit = (DeltaSeconds / timeUnit * 0.24) * gameSpeedMultiplier;
	float AddedClockwork = DeltaTimeUnit + clockwork;
	float NewDayTick = AddedClockwork / (60 * 24);
	float Remainder = FGenericPlatformMath::Fmod(AddedClockwork, (60.0f * 24.0f));
	clockwork = Remainder;
	dayTick = NewDayTick;
}

//Calculates time
void ATimeController::Clock() {
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

	//If the hour value has changed
	if (lastHour != hours) {

		//If running on the server
		if (HasAuthority()) {
			OnHourChanged();
		}
		lastHour = hours;
	}

	//Logs time and whether day or night
	//FString strHours = FString::FromInt(gameTime[2]);
	//FString HoursMinutesString = UKismetStringLibrary::BuildString_Int(strHours, ":", gameTime[1], "");
	//FString FinalString = UKismetStringLibrary::BuildString_Int(HoursMinutesString, ":", gameTime[0], "");
	//UE_LOG(LogTemp, Warning, TEXT("TimeController: Time: %s"), *FinalString);
	//UE_LOG(LogTemp, Warning, TEXT("Night: %s"), (bIsNight ? TEXT("True") : TEXT("False")));
}

//Calculates date
void ATimeController::Calendar() {
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

	//If the day value has changed
	if (lastDay != day) 
	{
		//If running on server
		if (HasAuthority())
		{
			OnDayChanged();
		}
		lastDay = day;
	}

	//Logs date
	//FString strDays = FString::FromInt(gameDate[0]);
	//FString DaysMonthString = UKismetStringLibrary::BuildString_Int(strDays, "/", gameDate[1], "");
	//FString FinalString = UKismetStringLibrary::BuildString_Int(DaysMonthString, "/", gameDate[2], "");
	//UE_LOG(LogTemp, Warning, TEXT("TimeController: Date: %s"), *FinalString);
}

void ATimeController::OnHourChanged()
{
	//Sync clockwork incase the clockwork has de-synced
	net_clockwork = clockwork;
	UE_LOG(LogTemp, Warning, TEXT("Hour Passed"))

	//Call blueprint function
	UpdateHour();
}

void ATimeController::OnDayChanged() 
{
	//Sync calendar incase of de-sync
	net_GameDate = gameDate;
	UE_LOG(LogTemp, Warning, TEXT("Day Passed"))

	//Call blueprint function
	UpdateDay();
}

void ATimeController::OnRep_Clockwork() 
{
	UE_LOG(LogTemp, Warning, TEXT("Syncing clockwork to: %f"), net_clockwork)
	clockwork = net_clockwork;
}

void ATimeController::OnRep_Calendar()
{
	UE_LOG(LogTemp, Warning, TEXT("Syncing date to: %d / %d / %d"), net_GameDate[0], net_GameDate[1], net_GameDate[2])
	gameDate = net_GameDate;
}

//Calculates SunAngle and returns to Time tick
FRotator ATimeController::SetDayNight() {
	float m_sunAngle = ((dayNightHours / 6) * 90) + 90;
	FRotator sunRot = UKismetMathLibrary::MakeRotator(180, m_sunAngle, 180 + sunRotationOffset);

	return sunRot;
}

//Calculates the sun intensity based on the height of the sun in the world
float ATimeController::CalculateSunIntensity() {
	float newIntensity = sunHeight * sunIntensityMultiplier;

	//Clamps value to make sure negative is not used
	if (newIntensity < 0) {
		newIntensity = 0.001f;
	}

	return newIntensity;
}