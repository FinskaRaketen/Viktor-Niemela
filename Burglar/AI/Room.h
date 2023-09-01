// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavMod.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Room.generated.h"

UCLASS()
class BURGLAR_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();
	UPROPERTY(Editanywhere)
		int RoomType;

	TArray<ANavMod*> mNavMods;

	UPROPERTY(Editanywhere)
		FName RoomTag;

	UPROPERTY(Editanywhere)
		UBoxComponent* BoxCollider;

	float TimeSinceSeenGreta;

	bool GetIsDownStairs() { return IsDownStairs; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	bool IsDownStairs;
};
