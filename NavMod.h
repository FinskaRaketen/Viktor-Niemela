// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavAreas/NavArea.h"
#include "Components/BoxComponent.h"
#include "NavMod.generated.h"

UCLASS()
class BURGLAR_API ANavMod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavMod();

	UPROPERTY(Editanywhere)
		UBoxComponent* BoxComponent;

	UPROPERTY(Editanywhere, Category = "Navigation")
		TSubclassOf<UNavArea> HighCostArea;

	UPROPERTY(Editanywhere, Category = "Navigation")
		TSubclassOf<UNavArea> LowCostArea;

	bool IsLowCost = true;

	void ChangeCost();

	UPROPERTY(Editanywhere)
		FName RoomTag;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
