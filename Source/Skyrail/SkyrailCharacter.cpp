// Copyright 2019 Ali El Saleh.

#include "SkyrailCharacter.h"
#include "SkyHook.h"
#include "Skyline.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ASkyrailCharacter::ASkyrailCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SkyHook = CreateDefaultSubobject<USkyHook>(FName("SkyHook"));

	this->AutoReceiveInput = EAutoReceiveInput::Player0;
	this->AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASkyrailCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASkyrailCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Play moving camera shakes if moving
	if (GetVelocity().Size() > 0.0f && CanJump())
	{
		// Play walk shake
		if (GetVelocity().Size() < GetCharacterMovement()->MaxWalkSpeed)
			UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(WalkShake);
		// Play run shake
		else
			UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(RunShake);
	}
	// Play idle camera shake if not moving
	else
		UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(IdleShake);
}

// Called to bind functionality to input
void ASkyrailCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis bindings
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASkyrailCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASkyrailCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASkyrailCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASkyrailCharacter::AddControllerPitchInput);

	// Action bindings
	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ASkyrailCharacter::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), IE_Released, this, &ASkyrailCharacter::StopJumping);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Pressed, this, &ASkyrailCharacter::Run);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Released, this, &ASkyrailCharacter::Walk);
	PlayerInputComponent->BindAction(FName("Escape"), IE_Pressed, this, &ASkyrailCharacter::Quit);
}

void ASkyrailCharacter::AddControllerPitchInput(const float Val)
{
	ACharacter::AddControllerPitchInput(Val);

	// Start raycasting when looking up and not hooked to the rail
	if (Controller->GetControlRotation().Pitch > 20.0f && Controller->GetControlRotation().Pitch < 90.0f && !bHooked && bCanLaunchToSkyrail)
	{
		// Create raycast data objects
		const FCollisionQueryParams CollisionParams;
		FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(100.0f, 1500.0f);

		// Calculate start and end traces
		//const FVector Center = GetActorLocation() + FVector(0, 0, 100); // Eye of character

		const FVector Start = GetActorLocation() + FVector(0, 0, 100); // Eye of character
		const FVector ForwardVector = Controller->GetControlRotation().Vector();
		//const FVector End = Center + ForwardVector * CollisionShape.GetCapsuleHalfHeight();
		const FVector End = Start + ForwardVector * SkyHook->Range;

		if (bDrawRaycast)
			DrawDebugLine(GetWorld(), Start, End, FColor::White);
			//DrawDebugCapsule(GetWorld(), End, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::MakeFromEuler(FVector(GetActorRotation().Roll, Controller->GetControlRotation().Pitch - 90, GetActorRotation().Yaw)), FColor::White, true, 1, 0, 3);

		const bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

		if (IsHit)
		{
			// If we hit something and it's a skyrail
			if (HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag(FName("Skyrail")))
			{
				if (bShowRaycastHits)
					DrawDebugPoint(GetWorld(), HitResult.Location, 5.0f, FColor::Green);

				if (bLogRaycastHits)
					UE_LOG(LogTemp, Warning, TEXT("Hit %s: %s"), *HitResult.GetActor()->Tags[0].ToString(), *HitResult.GetActor()->GetName());

				if (SkyHook)
				{
					SkyHook->Skyline = Cast<ASkyline>(HitResult.GetActor());
					SkyHook->PointOnRail = HitResult.Location;
				}
			}
		}
	}
	// Ray cast to the ground
	else if (Controller->GetControlRotation().Pitch < 340.0f && Controller->GetControlRotation().Pitch > 280.0f && bHooked)
	{
		// Create raycast data objects
		const FCollisionQueryParams CollisionParams;

		// Calculate start and end traces
		const FVector Start = GetActorLocation() + FVector(0, 0, 60); // Eye of character

		const FVector ForwardVector = Controller->GetControlRotation().Vector();
		const FVector End = Start + ForwardVector * SkyHook->Range;

		if (bDrawRaycast)
			DrawDebugLine(GetWorld(), Start, End, FColor::White, false, 1, 0, 1);

		const bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

		if (IsHit)
		{
			// If we hit something and it's the ground
			if (HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag(FName("Ground")))
			{
				if (bShowRaycastHits)
					DrawDebugPoint(GetWorld(), HitResult.Location, 5.0f, FColor::Green);

				if (bLogRaycastHits)
					UE_LOG(LogTemp, Warning, TEXT("Hit %s: %s"), *HitResult.GetActor()->Tags[0].ToString(), *HitResult.GetActor()->GetName());

				if (SkyHook)
					SkyHook->PointOnGround = HitResult.Location + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
			}
		}
	}
}

void ASkyrailCharacter::Landed(const FHitResult & Hit)
{
	Super::Landed(Hit);

	// Play jump shake but more intense to simulate landing effect
	UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(JumpShake, 2.5f);
}

void ASkyrailCharacter::MoveForward(const float AxisValue)
{
	FRotator ForwardRotation = Controller->GetControlRotation();

	// Limit pitch rotation
	if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		ForwardRotation.Pitch = 0.0f;

	// Find out which way is forward
	const FVector Direction = FRotationMatrix(ForwardRotation).GetScaledAxis(EAxis::X);

	// Apply movement in the calculated direction
	AddMovementInput(Direction, AxisValue);

	ForwardAxisValue = AxisValue;
}

void ASkyrailCharacter::MoveRight(const float AxisValue)
{
	// Find out which way is right
	const FRotator RightRotation = Controller->GetControlRotation();
	const FVector Direction = FRotationMatrix(RightRotation).GetScaledAxis(EAxis::Y);

	// Apply movement in the calculated direction
	AddMovementInput(Direction, AxisValue);
}

void ASkyrailCharacter::Walk()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASkyrailCharacter::Run()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ASkyrailCharacter::Quit()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, true);
}

void ASkyrailCharacter::Jump()
{
	ACharacter::Jump();

	// Play jump camera shake
	if (CanJump())
		UGameplayStatics::GetPlayerController(this, 0)->ClientPlayCameraShake(JumpShake, 2.0f);

	// Launch to skyrail
	if (SkyHook && HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag(FName("Skyrail")) && GetCharacterMovement()->IsMovingOnGround() && bCanLaunchToSkyrail)
	{
		bCanLaunchToSkyrail = false;
		SetActorEnableCollision(false);
		SkyHook->LaunchTowardsPointOnRail();
	}
	// Launch to ground
	else if (SkyHook && HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag(FName("Ground")) && bHooked)
	{
		bHooked = false;
		SetActorEnableCollision(true);
		SkyHook->LaunchTowardsPointOnGround();
	}
}
