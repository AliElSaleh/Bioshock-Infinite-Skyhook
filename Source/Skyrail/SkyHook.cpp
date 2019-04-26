// Copyright 2019 Ali El Saleh.

#include "SkyHook.h"
#include "SkyrailCharacter.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Skyline.h"

USkyHook::USkyHook()
{
	PrimaryComponentTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(FName("SkyHook"));
	SceneComponent->SetMobility(EComponentMobility::Stationary);
	SceneComponent->SetRelativeLocation(FVector(0, 0, 100));
}

void USkyHook::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ASkyrailCharacter>(GetOwner());
}

void USkyHook::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DistanceToRail = PointOnRail.Size() - Owner->GetActorLocation().Size();

	// Jump towards skyrail
	if (!Owner->bCanLaunchToSkyrail && !Owner->bHooked)
	{	
		Owner->SetActorLocation(FMath::Lerp(Owner->GetActorLocation(), PointOnRail, Owner->TimeToHookOnRail));
		
		if (DistanceToRail < 120.0f && !Owner->bCanLaunchToSkyrail && !Owner->bHooked)
		{
			Owner->bHooked = true;
			GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(Owner->HookShake, 10.0f);

			// Ride the skyline
			if (Skyline)
			{
				const float Point = Skyline->SplineComponent->FindInputKeyClosestToWorldLocation(Owner->GetActorLocation());

				Skyline->TimelineComponent->SetNewTime(Point);
				Skyline->TimelineComponent->Play();
			}
		}
	}
	// Jump towards ground
	else if (Owner->bCanLaunchToGround && !Owner->bHooked)
	{
		if (!Owner->GetCharacterMovement()->IsMovingOnGround())
			Owner->SetActorLocation(FMath::Lerp(Owner->GetActorLocation(), PointOnGround, Owner->TimeToHookOnRail));
		else
			Owner->bCanLaunchToGround = false;
	}
}  

void USkyHook::LaunchTowardsPointOnRail() const
{
	UE_LOG(LogTemp, Warning, TEXT("SkyHook: Launching to skyrail at %s"), *PointOnRail.ToString());

	Owner->bCanLaunchToGround = false;
	Owner->bHooked = false;

	// Calculate the launch velocity
	//const FVector XVelocity = Owner->GetActorForwardVector() * DistanceToRail * (DistanceToRail < 900.0f ? 0.3f : 1.0f);
	//const FVector ZVelocity = Owner->GetActorUpVector() * DistanceToRail * (DistanceToRail < 1000.0f ? 2.0f : 1.1f);
	//FVector LaunchVelocity = XVelocity + ZVelocity;
	//
	//UE_LOG(LogTemp, Warning, TEXT("SkyHook: LaunchVelocity %s"), *LaunchVelocity.ToString());

	//Owner->LaunchCharacter(LaunchVelocity, true, true);
}

void USkyHook::LaunchTowardsPointOnGround() const
{
	UE_LOG(LogTemp, Warning, TEXT("SkyHook: Launching to ground at %s"), *PointOnGround.ToString());

	Skyline->TimelineComponent->Stop();

	Owner->bCanLaunchToGround = true;
	Owner->bCanLaunchToSkyrail = true;
}
