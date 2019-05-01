// Copyright 2019 Ali El Saleh.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SplineMeshComponent.h"
#include "Skyline.generated.h"

UCLASS()
class SKYRAIL_API ASkyline : public AActor
{
	GENERATED_BODY()
	
public:	
	ASkyline();

	void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* TimelineComponent = nullptr;

	UPROPERTY(EditAnywhere)
	USplineComponent* SplineComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<USplineMeshComponent*> SplinePoints;

protected:
	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	UFUNCTION()
	void MovePlayerAlongSkyline();

	UStaticMesh* RailMesh = nullptr;

	UPROPERTY(EditAnywhere)
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class ASkyrailCharacter* Player = nullptr;

	UPROPERTY(EditAnywhere)
	UMaterial* Material = nullptr;

	UPROPERTY(VisibleAnywhere)
	UCurveFloat* AnimAlpha = nullptr;

	UPROPERTY(EditAnywhere)
	float Speed = 1.0f;
};
