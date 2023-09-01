// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AITypes.h"
#include "AIController.h"
#include <GameFramework/FloatingPawnMovement.h>
#include "Components/CapsuleComponent.h"
#include <Navigation/PathFollowingComponent.h>
#include "GretaAiController.h"
#include "../Player/VRPlayer.h"
#include "Room.h"
#include "NavigationSystem.h"
#include "Magda.h"
#include "../Interactables/DoorClass.h"
#include "Components/SphereComponent.h"
#include "Greta.generated.h"

UENUM()
enum class State
{
	PATROL,
	INVESTIGATION,
	HUNTING,
	SNEAK,
};

USTRUCT()
struct FNeed
{

	GENERATED_BODY()

	FNeed()
	{
		
	}
	FNeed(FString Name, float Need_Value, float Need_IncreaseRate, float Need_IncreaseTimer, float Need_DecreaseRoomPresence, float Need_DecreaseTimer, int Room_Type)
	{
		NeedName = Name;
		NeedValue = Need_Value;
		NeedIncreaseRate = Need_IncreaseRate;
		NeedIncreaseTimer = Need_IncreaseTimer;
		NeedDecreaseRoomPresence = Need_DecreaseRoomPresence;
		NeedDecreaseTimer = Need_DecreaseTimer;
		RoomType = Room_Type;
		OrigianlDecreaseTimer = Need_DecreaseTimer;
		OriginalIncreaseTimer = Need_IncreaseTimer;
	}

	UPROPERTY(Editanywhere)
	FString NeedName;

	UPROPERTY(Editanywhere)
	float NeedValue;

	UPROPERTY(Editanywhere)
	float NeedIncreaseRate;

	UPROPERTY(Editanywhere)
	float NeedIncreaseTimer;

	UPROPERTY(Editanywhere)
	float NeedDecreaseRoomPresence;

	UPROPERTY(Editanywhere)
	float NeedDecreaseTimer;

	UPROPERTY(VisibleAnywhere)
	int RoomType;

	UPROPERTY()
	int OriginalIncreaseTimer;
	UPROPERTY()
	int OrigianlDecreaseTimer;
};

UCLASS()
class BURGLAR_API AGreta : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGreta();

	bool IsPlayerInVisibleRange();

	void StateMachine(float DeltaTime);

	void Patrol(float DeltaTime);

	void Hunting(float DeltaTime);

	int NeedCalculation();

	void GetNewMovePoint();

	FVector SelectRoom(TArray<ARoom*> Rooms, int RoomType);

	float AddRoomPresence(ARoom* Room);

	ARoom* IsInRoom();

	void Sneaking(float DeltaTime);

	void Investigating(float DeltaTime);

	void Action();

	void OffSetLocation();

	void PatrolFloor(TArray<ARoom*> Rooms, float DeltaTime);

	void SneakCalculation(TArray<ARoom*> Rooms);

	void InvestigationCalculation(TArray<ARoom*> Rooms);

	void OpenDoors(float DeltaTime);

	void NewRandomMoveTo();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnBeginHunt();
	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnEndHunt();

	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void OnHeardSound();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void WhileUnawarePatrol();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void OnTellMagdaToSearch();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void WhileSearching();
	UFUNCTION(BlueprintImplementableEvent, Category = "SoundEvent")
	void OnHitByPlayer();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnOpenDoor();
	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void OnAttack();

	UFUNCTION()
	void NotifyPlayerAttack( UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector HitLocation, const FHitResult& Hit );

	UFUNCTION()
	void OnDoorColliderBeginOverlap( UPrimitiveComponent*   OverlappedComponent
								 	 , AActor*              OtherActor
								 	 , UPrimitiveComponent* OtherComp
								 	 , int32                OtherBodyIndex
								 	 , bool                 bFromSweep
								 	 , const FHitResult&    SweepResult );

	UPROPERTY()
	float SoundTimer;
	UPROPERTY()
	float TimePerSoundCall;
	UPROPERTY(EditDefaultsOnly, Category = "SoundTimer");
	float MinTimePerSoundCall;
	UPROPERTY(EditDefaultsOnly, Category = "SoundTimer");
	float MaxTimePerSoundCall;
	
	FAIMoveRequest			mMoveRequest;

	UPROPERTY()
	AGretaAiController* AIController;

	UPROPERTY()
	UNavigationSystemV1* mpNavMesh;

	UPROPERTY(Visibleanywhere, Category= "Patrol")
	float Patroltimer;
	
	float VisibleTimer;

	UPROPERTY(EditDefaultsOnly)
	UFloatingPawnMovement* mpFloatingPawnMovement;

	UPROPERTY(EditDefaultsOnly)
		UCapsuleComponent* CapsuleCollider;

	UPROPERTY(EditAnywhere)
		UBoxComponent* DoorCollider;

	UPROPERTY(EditDefaultsOnly)
		USphereComponent* OverlapDoorSphereCollider;

	float mHearingRange;

	UPROPERTY(Editanywhere, Category = "Hunting")
	float VisibleRange;

	UPROPERTY(Editanywhere, Category = "Collision")
		bool ShouldOpenClosedDoors;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	UCapsuleComponent* ColliderForPlayerAttacks;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	AVRPlayer* Player;
	UClass* mPRoomClass;
	State CurrentState;
	UPROPERTY(Editanywhere, Category= "Room")
	TArray<FNeed> Needs;
	FPathFollowingRequestResult mMoveRequestResult;

	UPROPERTY(VisibleAnywhere, Category= "Room")
	TArray<ARoom*> HouseRooms;

	UPROPERTY(VisibleAnywhere, Category = "Room")
	TArray<ARoom*> BasementRooms;

	UPROPERTY(Editanywhere, Category= "Movement")
	float mMovementSpeed;

	UPROPERTY(Editanywhere, Category= "Movement")
	float OrginalMovementSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Aware")
		bool Aware;

	UPROPERTY(Editanywhere, Category = "Movement")
		float MaxStamina = 40.0f;

	float Stamina;

	UPROPERTY(Editanywhere, Category = "Movement")
		float StaminaDecay;

	float HuntingLookAroundTimer;

	bool Walking;

	bool Running;

	UPROPERTY(Editanywhere, Category = "Movement")
		float StaminaIncrease;

	FVector TargetPoint;

	UPROPERTY(Editanywhere, Category = "Movement")
		float RotationSpeed;

	FVector LastPosition;

	TArray<AActor*> ActionItems;

	FVector HearingLocation;

	bool HeardSound;

	UPROPERTY(Editanywhere, Category= "AIHearing")
	float HeardSoundLifeTime;
	UPROPERTY(Editanywhere, Category = "AIHearing")
		int HeardNoicesCount;

	float AwareTimer;

	UPROPERTY(Editanywhere, Category= "Aware")
	float InitialAwareTimerValue;

	bool PlayerIsSeen;

	float InitialHearingRange;
	
	int InitialHeardNoicesCount;

	float TimeSinceUpdatingRooms;

	bool SneakCalulation;

	bool InvestigationCaluclation;

	AMagda* Magda;

	float SoundLifeTime;

	bool IsPlayerInBasement;

	ADoorClass* CollidedDoor;

	bool DoorRotateBack;

	bool IsDoorLocked;

	FRotator InitialDoorRot;

	float DoorInterpolationDuration;

	UPROPERTY(EditAnywhere,Category = "Movement")
	bool Stunned;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StunnedDuration;

	float InitialStunnedDuration;

	UPROPERTY(EditAnywhere, Category = "Aware")
		float MagdaSearchingTimer;

	float InitialMagdaSearchingTimer;

	bool RestlessRunning;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float MovementSpeedIncreaseMultiplier;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float MovementSpeedStartIncreaseMultiplier;

};
