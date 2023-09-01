// Fill out your copyright notice in the Description page of Project Settings.


#include "SafeInteractComponent.h"
#include "Kismet/GameplayStatics.h"

USafeInteractComponent::USafeInteractComponent()
	:CurrentTargetRot(0.0f)
	,SetTargetRotationTimer(0.5f)
{
	
}

void USafeInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if (!RootComponent)
		return;

	RootComponent->SetCollisionProfileName("Grabbable");
	RootComponent->SetGenerateOverlapEvents(true);

	AActor* FoundActor;
	FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASafePuzzleSystem::StaticClass());

	SafeActor = Cast<ASafePuzzleSystem>(FoundActor);

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "DoorKnob", FoundActors);

	SafeKnobActor = FoundActors[0];
}

void USafeInteractComponent::Update(UPhysicsHandleComponent* PhysicsHandle, const FVector TargetLocation, const FRotator TargetRotation)
{
	if (CurrentTargetRot == FRotator(0.0f))
		CurrentTargetRot = TargetRotation;

	UPrimitiveComponent* ComponentTest = Cast<UPrimitiveComponent>(SafeKnobActor->GetRootComponent());
	UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(SafeKnobActor->GetRootComponent());
	
	Component->SetSimulatePhysics(false);

	float Dir = CurrentTargetRot.Yaw - TargetRotation.Yaw;
	
	if(Dir > 2.5)
	{
		if(Dir == 0)
		{
			Dir = 5.0f;
		}
		PhysicsHandle->ReleaseComponent();
		SafeKnobActor->AddActorLocalRotation(FRotator(-5.0f, 0.0f, 0.0f));
		CurrentTargetRot = TargetRotation;
		PhysicsHandle->GrabComponentAtLocation(Component, "", Component->GetComponentLocation());
		SafeActor->AddRotationNumber(-1);
	}
	else if(Dir < -2.5)
	{
		if (Dir == 0)
		{
			Dir = -5.0f;
		}
		PhysicsHandle->ReleaseComponent();
		SafeKnobActor->AddActorLocalRotation(FRotator(5.0f, 0.0f, 0.0f));
		CurrentTargetRot = TargetRotation;
		PhysicsHandle->GrabComponentAtLocation(Component, "", Component->GetComponentLocation());
		SafeActor->AddRotationNumber(1);
	}
	
	
}

void USafeInteractComponent::OnDrop()
{
	Super::OnDrop();

	CurrentTargetRot = FRotator(0.0f);

}

void USafeInteractComponent::OnPickup()
{
	Super::OnPickup();
}
