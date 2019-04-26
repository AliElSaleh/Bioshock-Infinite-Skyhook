// Copyright 2019 Ali El Saleh.

#pragma once

#include "Components/ActorComponent.h"
#include "SkyHook.generated.h"

UCLASS( ClassGroup=(Character), meta=(BlueprintSpawnableComponent) )
class SKYRAIL_API USkyHook : public UActorComponent
{
	GENERATED_BODY()

public:	
	USkyHook();

	void LaunchTowardsPointOnRail() const;
	void LaunchTowardsPointOnGround() const;

	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComponent = nullptr;

	FVector PointOnRail = FVector(0, 0, 0);
	FVector PointOnGround = FVector(0, 0, 0);

	class ASkyline* Skyline = nullptr;

protected:
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	class ASkyrailCharacter* Owner = nullptr;

	float DistanceToRail = 0.0f;
};
