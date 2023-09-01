// Fill out your copyright notice in the Description page of Project Settings.


#include "Burglar/AI/Room.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ANavMod::StaticClass(), RoomTag, FoundActors);

	for(int i = 0; i < FoundActors.Num(); i++)
	{
		ANavMod* Temp = Cast<ANavMod>(FoundActors[i]);
		mNavMods.Add(Temp);
	}

	if(GetActorLocation().Z < 18600)
	{
		IsDownStairs = true;
	}
	else
	{
		IsDownStairs = false;
	}
}

// Called every frame
void ARoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

