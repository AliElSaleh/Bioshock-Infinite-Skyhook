// Copyright 2019 Ali El Saleh.

#include "SkyHook.h"
#include "SkyrailCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Skyline.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"

USkyHook::USkyHook()
{
	PrimaryComponentTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SkyHook"));
	SceneComponent->SetMobility(EComponentMobility::Stationary);
	SceneComponent->SetRelativeLocation(FVector(0, 0, 100));

	// Timeline
	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(FName("Timeline"));
	AnimAlpha = Cast<UCurveFloat>(StaticLoadObject(UCurveFloat::StaticClass(), nullptr, TEXT("CurveFloat'/Game/Curves/SkyhookCurve.SkyhookCurve'")));
}

void USkyHook::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ASkyrailCharacter>(GetOwner());
	AnimAlpha = Cast<UCurveFloat>(StaticLoadObject(UCurveFloat::StaticClass(), nullptr, TEXT("CurveFloat'/Game/Curves/SkyhookCurve.SkyhookCurve'")));

	// Timeline Initialization
	FOnTimelineFloat TimelineCallback;
	TimelineCallback.BindUFunction(this, "MovePlayerToPointOnCurve");

	if (AnimAlpha)
	{
		TimelineComponent = NewObject<UTimelineComponent>(this, FName("Timeline"));
		TimelineComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		TimelineComponent->SetPropertySetObject(this);
		TimelineComponent->SetLooping(false);
		TimelineComponent->SetPlaybackPosition(0.0f, false, false);
		TimelineComponent->SetPlayRate(1.0f);
		TimelineComponent->AddInterpFloat(AnimAlpha, TimelineCallback);
		TimelineComponent->SetTimelineLength(Owner->TimeToHookAndLand);
		TimelineComponent->SetTimelineLengthMode(TL_TimelineLength);
		TimelineComponent->RegisterComponent();

		AnimAlpha->ResetCurve();
		AnimAlpha->FloatCurve.AddKey(0.0f, 0.0f);
		AnimAlpha->FloatCurve.AddKey(Owner->TimeToHookAndLand, 1.0f);
	}
}

void USkyHook::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TimelineComponent)
		TimelineComponent->TickComponent(DeltaTime, LEVELTICK_TimeOnly, nullptr);

	DistanceToRail = FVector::Dist(Owner->GetActorLocation(), PointOnRail);
	DistanceToGround = FVector::Dist(Owner->GetActorLocation(), PointOnGround);

	//UE_LOG(LogTemp, Warning, TEXT("Distance to rail: %f"), DistanceToRail)
	//UE_LOG(LogTemp, Warning, TEXT("Distance to ground: %f"), DistanceToGround)

	// Jump towards skyrail
	if (!Owner->bCanLaunchToSkyrail && !Owner->bHooked)
	{
		if (DistanceToRail < 10.0f && !Owner->bCanLaunchToSkyrail && !Owner->bHooked)
		{
			Owner->bHooked = true;
			TimelineComponent->Stop();

			// Play the hook camera shake
			GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(Owner->HookShake, 10.0f);

			// Ride the skyline
			if (Skyline)
			{
				const float Point = Skyline->SplineComponent->FindInputKeyClosestToWorldLocation(PointOnRail);

				Skyline->TimelineComponent->SetNewTime(Point);
				Skyline->TimelineComponent->Play();
			}
		}
	}
	// Jump towards ground
	else if (Owner->bCanLaunchToGround && !Owner->bHooked)
	{
		if (Owner->GetCharacterMovement()->IsMovingOnGround())
			Owner->bCanLaunchToGround = false;
	}
}

void USkyHook::MovePlayerToPointOnCurve()
{
	const float Time = AnimAlpha->GetFloatValue(TimelineComponent->GetPlaybackPosition());

	// Create points on the curve
	const FVector D = FMath::Lerp(A, B, Time);
	const FVector E = FMath::Lerp(B, C, Time);
	
	const FVector PointOnCurve = FMath::Lerp(D, E, Time);

	Owner->SetActorLocation(PointOnCurve);
	
	DrawDebugPoint(GetWorld(), PointOnCurve, 10.0f, FColor::White, true, 10.0f);
}

void USkyHook::LaunchTowardsPointOnRail()
{
	UE_LOG(LogTemp, Warning, TEXT("SkyHook: Launching to skyrail at %s"), *PointOnRail.ToString());

	TimelineComponent->SetPlayRate(DistanceToRail < 1500.0f ? 1.5f : 1.0f);

	Owner->bCanLaunchToGround = false;
	Owner->bHooked = false;

	// Create a bezier curve
	FVector Point = Owner->GetActorLocation() + Owner->GetActorForwardVector() * DistanceToRail/2.0f;
	Point.Z = PointOnRail.Z;

	A = Owner->GetActorLocation();
	B = Point;
	C = PointOnRail;

	if (Owner->bDrawRaycast)
	{
		DrawDebugPoint(GetWorld(), Point, 10.0f, FColor::Red, true, 10.0f);
		DrawDebugLine(GetWorld(), A, B, FColor::Green, true, 10, 0, 5.0f);
		DrawDebugLine(GetWorld(), B, C, FColor::Green, true, 10, 0, 5.0f);
	}

	TimelineComponent->PlayFromStart();
}

void USkyHook::LaunchTowardsPointOnGround()
{
	UE_LOG(LogTemp, Warning, TEXT("SkyHook: Launching to ground at %s"), *PointOnGround.ToString());

	TimelineComponent->SetPlayRate(DistanceToGround < 1300.0f ? 1.5f : 0.7f);

	Skyline->TimelineComponent->Stop();

	Owner->bCanLaunchToGround = true;
	Owner->bCanLaunchToSkyrail = true;

	// Create a bezier curve
	FVector Point = Owner->GetActorLocation() + Owner->GetActorForwardVector() * DistanceToGround/2.0f;
	Point.Z = PointOnRail.Z;

	A = Owner->GetActorLocation();
	B = Point;
	C = PointOnGround;

	if (Owner->bDrawRaycast)
	{
		DrawDebugPoint(GetWorld(), Point, 10.0f, FColor::Red, true, 10.0f);
		DrawDebugLine(GetWorld(), A, B, FColor::Green, true, 10, 0, 5.0f);
		DrawDebugLine(GetWorld(), B, C, FColor::Green, true, 10, 0, 5.0f);
	}

	TimelineComponent->PlayFromStart();
}
