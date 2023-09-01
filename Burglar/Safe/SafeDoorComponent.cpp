// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeDoorComponent.h"
#include "MotionControllerComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USafeDoorComponent::Update(UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation)
{
	
	if(Locked == false)
	{

		OwnerActor->SetSimulatePhysics(false);
		FRotator Temp = UKismetMathLibrary::FindLookAtRotation(OwnerActor->GetComponentLocation(), TargetLocation);
		Temp.Yaw -= GrabOffset;
		Temp.Yaw =  FMath::Clamp(Temp.Yaw, MinRotationValue,MaxRotationValue);
		PhysicsHandle->ReleaseComponent();
		if (OwnerActor->GetComponentRotation().Yaw == MaxRotationValue && Temp.Yaw <= MinRotationValue)
			return;
		OwnerActor->SetWorldRotation(FRotator(0, Temp.Yaw ,0));
	}
}

void USafeDoorComponent::OnPickup()
{
	Super::OnPickup();

	FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerActor->GetComponentLocation(), GrabbingController->GetComponentLocation());
	GrabOffset = (LookAt - OwnerActor->GetComponentRotation()).Yaw;
}

void USafeDoorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	MaxRotationValue = OwnerActor->GetComponentRotation().Yaw;
	MinRotationValue = MaxRotationValue - 178;

	BeginOpeningSafeRotation = OwnerActor->GetComponentRotation();
	BeginOpeningSafeRotation.Yaw -= 10.0f;
}
