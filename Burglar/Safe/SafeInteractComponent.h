// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.h"
#include "SafePuzzleSystem.h"
#include "SafeInteractComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BURGLAR_API USafeInteractComponent : public UGrabbableComponent
{
	GENERATED_BODY()

public:

	USafeInteractComponent();

	void BeginPlay() override;

	void Update(UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation) override;
	
	void OnDrop() override;

	void OnPickup() override;
private:
	ASafePuzzleSystem* SafeActor;
	AActor* SafeKnobActor;
	FRotator CurrentTargetRot;
	float SetTargetRotationTimer;
	bool InvertedRotation;
};
