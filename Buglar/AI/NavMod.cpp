// Fill out your copyright notice in the Description page of Project Settings.


#include "Burglar/AI/NavMod.h"

// Sets default values
ANavMod::ANavMod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("Collider");
	SetRootComponent(BoxComponent);
	BoxComponent->SetAreaClassOverride(LowCostArea);
}

void ANavMod::ChangeCost()
{
	if(IsLowCost)
	{
		BoxComponent->SetAreaClassOverride(HighCostArea);
		IsLowCost = false;
	}
	else
	{
		BoxComponent->SetAreaClassOverride(LowCostArea);
	}
}

// Called when the game starts or when spawned
void ANavMod::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANavMod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

