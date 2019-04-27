// Copyright (c) 2019 Ali El Saleh.

#pragma once

#include "GameFramework/Character.h"
#include "SkyrailCharacter.generated.h"

UCLASS()
class SKYRAIL_API ASkyrailCharacter final : public ACharacter
{
	GENERATED_BODY()

public:
	ASkyrailCharacter();

	bool bCanLaunchToSkyrail = true;
	bool bCanLaunchToGround = false;
	bool bHooked = false;

	UPROPERTY(EditAnywhere, meta=(ToolTip = "How long should the player take to reach the skyrail?"))
	float TimeToHookOnRail = 5.0f;

	UPROPERTY(EditAnywhere, meta=(ToolTip = "How long should the player take to reach the ground?"))
	float TimeToLandOnGround = 5.0f;

	UPROPERTY(EditAnywhere)
	float SkylineSpeed = 1.0f;

	float ForwardAxisValue = 0.0f;
	
	FHitResult HitResult;

protected:
	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Jump() override;
	void AddControllerPitchInput(float Val) override;
	void Landed(const FHitResult& Hit) override;

	UFUNCTION()
	void MoveForward(float AxisValue);

	UFUNCTION()
	void MoveRight(float AxisValue);

	// Set walk speed
	UFUNCTION()
	void Walk();

	// Set run speed
	UFUNCTION()
	void Run();

	// Quit game
	UFUNCTION()
	void Quit();

	class USkyHook* SkyHook = nullptr;

	UPROPERTY(EditAnywhere)
	float WalkSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere)
	float RunSpeed = 800.0f;

	UPROPERTY(EditAnywhere)
	bool bDrawRaycast = false;

	UPROPERTY(EditAnywhere)
	bool bShowRaycastHits = false;

	UPROPERTY(EditAnywhere)
	bool bLogRaycastHits = false;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> IdleShake = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> WalkShake = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> RunShake = nullptr;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> JumpShake = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShake> HookShake = nullptr;
};
