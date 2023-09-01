// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include <GameFramework/FloatingPawnMovement.h>
#include "Components/CapsuleComponent.h"
#include <Navigation/PathFollowingComponent.h>
#include "GretaAiController.h"
#include <Components/SkeletalMeshComponent.h>
#include "Components/SphereComponent.h"
#include "NavigationSystem.h"

#include "Magda.generated.h"

UENUM()
enum class MagdaState
{
	Sleeping,
	Passive,
	Bark,
	Search,
};


UCLASS()
class BURGLAR_API AMagda : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMagda();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void WhileHoldingBone();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void OnSeePlayerWithBone();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void WhileBarking();

	void StateMachine(float DeltaTime);

	void Sleeping(float DeltaTime);

	void Passive(float DeltaTime);

	void Bark(float DeltaTime);

	void Search(float DeltaTime);

	bool IsPlayerInVisibleRange(float Range, bool ThroughWalls);

	void IsOnGround();

	bool AlreadyOnSleepingPos();

	UPROPERTY()
		UNavigationSystemV1* NavMesh;

	UPROPERTY()
	AGretaAiController* AIController;

	FAIMoveRequest	MoveRequest;

	FPathFollowingRequestResult MoveRequestResult;

	TArray<AActor*> SleepingPositions;

	UPROPERTY()
	float SoundTimer;
	UPROPERTY()
	float TimePerSoundCall;
	UPROPERTY(EditDefaultsOnly, Category = "SoundTimer");
	float MinTimePerSoundCall;
	UPROPERTY(EditDefaultsOnly, Category = "SoundTimer");
	float MaxTimePerSoundCall;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Visibleanywhere, Category = "Aware")
	bool Awake;

	UPROPERTY(Editanywhere, Category = "Aware")
	float AwakeDuration;

	float InitialAwakeDuration;

	UPROPERTY(Editanywhere, Category = "Movement")
	float MovementSpeed;

	float InitialMovementSpeed;

	UPROPERTY(Editanywhere, Category = "Aware")
		float AwakeRange;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* SkeletalMesh;

	UPROPERTY(EditDefaultsOnly)
	UCapsuleComponent* CapsuleCollider;

	UPROPERTY(EditAnywhere)
	USphereComponent* BoneCollider;

	UFloatingPawnMovement* FloatingPawnMovement;

	UPROPERTY(VisibleAnywhere, Category = "Player")
	AActor* PlayerActor;

	MagdaState CurrentState;

	UPROPERTY(Editanywhere, Category = "Aware")
		float VisibleRange;

	UPROPERTY(Editanywhere, Category = "Bark")
		float BarkDuration;

	UPROPERTY(Editanywhere, Category = "Search")
		float SearchRadius;

	UPROPERTY(Editanywhere, Category = "Passive")
		float PassiveRadius;

	float IntialBarkDuration;

	bool StartSearching;

	UPROPERTY(Editanywhere, Category = "Search")
		float SearchIdleDuration;

	UPROPERTY(Editanywhere, Category = "Search")
		float SearchDuration;

	UPROPERTY(Editanywhere, Category = "Search")
		float IncreaseMovementSpeedMultiplier;

	UPROPERTY(Editanywhere, Category = "Search")
		float StartBarkingDuration;

	bool ShouldStartBarking;

	/*
	UPROPERTY(Editanywhere, Category = "TimeSinceDurations")
		float TimeSinceBarked;

	UPROPERTY(Editanywhere, Category = "TimeSinceDurations")
		float TimeSinceSearched;	
		*/
	float PassiveIdleDuration;

	UPROPERTY(Editanywhere, Category = "Passive")
		int PassiveIdleDurationMaxValue;

	UPROPERTY(Editanywhere, Category = "Passive")
		int PassiveIdleDurationMinimumValue;

	bool IsBarking;

	UPROPERTY(Editanywhere, Category = "Movement")
		float RotationSpeed;

private:
	float InitialSearchIdleDuration;
	float InitialSearchDuration;
	float InitialStartBarkingDuration;
	float InitialPassiveIdleDuration;
	FVector LastPos;
	FVector TargetPoint;

	UPROPERTY(VisibleAnywhere)
		bool HasReceivedBone;

};
