// Copyright 2019 Ali El Saleh.

#pragma once

#include "Components/ActorComponent.h"
#include "Float.generated.h"

/*
	Makes an actor float up and down
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKYRAIL_API UFloat : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFloat();

protected:
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float RunningTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float XValue;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float YValue;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float ZValue;

	AActor* Owner = nullptr;
};
