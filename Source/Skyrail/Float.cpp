// Copyright 2019 Ali El Saleh.

#include "Float.h"
#include "GameFramework/Actor.h"

UFloat::UFloat()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFloat::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	Owner->GetRootComponent()->SetMobility(EComponentMobility::Movable);
}

void UFloat::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector NewLocation;
	const float DeltaHeight = FMath::Sin(RunningTime+DeltaTime) - FMath::Sin(RunningTime);
	NewLocation.X = DeltaHeight * XValue;
	NewLocation.Y = DeltaHeight * YValue;
	NewLocation.Z = DeltaHeight * ZValue;

	RunningTime += DeltaTime;

	Owner->SetActorLocation(Owner->GetActorLocation() + NewLocation);
}

