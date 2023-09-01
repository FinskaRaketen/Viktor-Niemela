// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "SafeDoorComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BURGLAR_API USafeDoorComponent : public UGrabbableComponent
{
	GENERATED_BODY()

	void Update(UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation) override;

	void OnPickup() override;

	void BeginPlay() override;
public:
	void UnLocked() { Locked = false; }
	float GetMaxRotationValue() { return MaxRotationValue; }
	float GetMinRotationValue() { return MinRotationValue; }
	FRotator GetTargetSafeRotation() { return BeginOpeningSafeRotation; }
private:
	bool Locked = true;
	UPrimitiveComponent* OwnerActor;

	float GrabOffset;
	float MaxRotationValue;
	float MinRotationValue;

	FRotator BeginOpeningSafeRotation;
};
