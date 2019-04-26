// Copyright 2019 Ali El Saleh.

#include "Skyline.h"
#include "SkyrailCharacter.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"

ASkyline::ASkyline()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add("Skyrail");

	SceneComponent = CreateDefaultSubobject<USceneComponent>(FName("Scene"));
	SceneComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
	RootComponent = SceneComponent;

	// Spline Component
	SplineComponent = CreateDefaultSubobject<USplineComponent>(FName("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->SetMobility(EComponentMobility::Static);
	RootComponent->SetMobility(EComponentMobility::Static);
	SetActorEnableCollision(true);

	RailMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'")));
	Material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/Game/Materials/SkylineMaterial.SkylineMaterial'")));

	// Timeline
	TimelineComponent = CreateDefaultSubobject<UTimelineComponent>(FName("Timeline"));
	AnimAlpha = CreateDefaultSubobject<UCurveFloat>("AnimAlpha");
}

void ASkyline::OnConstruction(const FTransform& Transform)
{
	FVector StartLocation, StartTangent;
	FVector EndLocation, EndTangent;

	SplinePoints.Empty();
	
	if (SplineComponent->GetNumberOfSplinePoints() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline has too few points."));
		return;
	}

	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints() - 1; i++)
	{
		// Create a new spline mesh
		auto* SplineMesh = NewObject<USplineMeshComponent>(this);
		SplineMesh->RegisterComponent();
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;

		SplineComponent->GetLocationAndTangentAtSplinePoint(i, StartLocation, StartTangent, ESplineCoordinateSpace::Local);
		SplineComponent->GetLocationAndTangentAtSplinePoint(i + 1, EndLocation, EndTangent, ESplineCoordinateSpace::Local);

		// Set Spline mesh settings
		SplineMesh->SetMobility(EComponentMobility::Static);
		SplineMesh->ForwardAxis = ESplineMeshAxis::Z;
		SplineMesh->SetCollisionObjectType(ECC_WorldStatic);
		SplineMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SplineMesh->SetCollisionResponseToAllChannels(ECR_Block);

		SplineMesh->SetStaticMesh(RailMesh);
		SplineMesh->SetMaterial(0, Material);

		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		// Add to the spline point
		SplineMesh->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent);

		SplinePoints.Add(SplineMesh);
	}
}

void ASkyline::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASkyrailCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	Speed = Player->SkylineSpeed;

	// Timeline Initialization
	FOnTimelineFloat TimelineCallback;
	TimelineCallback.BindUFunction(this, "MovePlayerAlongSkyline");

	if (AnimAlpha != nullptr)
	{
		TimelineComponent = NewObject<UTimelineComponent>(this, FName("Timeline"));
		TimelineComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		TimelineComponent->SetPropertySetObject(this);
		TimelineComponent->SetLooping(false);
		TimelineComponent->SetPlaybackPosition(0.0f, false, false);
		TimelineComponent->SetPlayRate(Speed);
		TimelineComponent->AddInterpFloat(AnimAlpha, TimelineCallback);
		TimelineComponent->SetTimelineLength(SplinePoints.Num());
		TimelineComponent->SetTimelineLengthMode(TL_TimelineLength);
		TimelineComponent->RegisterComponent();

		AnimAlpha->ResetCurve();
		AnimAlpha->FloatCurve.AddKey(0, 0);
		AnimAlpha->FloatCurve.AddKey(SplinePoints.Num(), 1);
	}
}

void ASkyline::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TimelineComponent)
		TimelineComponent->TickComponent(DeltaTime, LEVELTICK_TimeOnly, nullptr);
}

void ASkyline::MovePlayerAlongSkyline()
{
	const float Time = AnimAlpha->GetFloatValue(TimelineComponent->GetPlaybackPosition());

	// Move the player
	if (Player)
	{
		Player->SetActorTransform(FTransform(
			SplineComponent->GetWorldRotationAtTime(Time),
			SplineComponent->GetWorldLocationAtTime(Time) - FVector(0, 0, 130)
		));

		// Speed up/slow down controls on skyline
		if (Player->ForwardAxisValue > 0.0f)
		{
			Speed += 0.02f;
			Speed = FMath::Clamp(Speed, 1.0f, 5.0f);
			TimelineComponent->SetPlayRate(Speed);
		}
		else if (Player->ForwardAxisValue < 0.0f)
		{
			Speed -= 0.02f;
			Speed = FMath::Clamp(Speed, 1.0f, 5.0f);
			TimelineComponent->SetPlayRate(Speed);
		}
	}

	//	Release player at end of spline
	if (TimelineComponent->GetPlaybackPosition() >= TimelineComponent->GetTimelineLength())
	{
		Player->bHooked = false;
		Player->bCanLaunchToGround = false;
		Player->bCanLaunchToSkyrail = true;
		Player->SetActorEnableCollision(true);
	}
}

