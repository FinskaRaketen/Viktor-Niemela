// Fill out your copyright notice in the Description page of Project Settings.


#include "Greta.h"
#include <UObject/ConstructorHelpers.h>
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGreta::AGreta()
	: SoundTimer(0)
	, TimePerSoundCall(.5f)
	, MinTimePerSoundCall(.5f)
	, MaxTimePerSoundCall(1)
    , mMoveRequest(FAIMoveRequest())
	, AIController(nullptr)
	, VisibleTimer(0.6f)
	, mHearingRange(800.0f)
	, VisibleRange(800.0f)
	, ShouldOpenClosedDoors(false)
	, ColliderForPlayerAttacks(nullptr)
	, Player(nullptr)
	, CurrentState(State::PATROL)
	, mMoveRequestResult()
	, mMovementSpeed(5.0f)
	, OrginalMovementSpeed()
	, Aware(false)
	, Stamina(MaxStamina)
	, StaminaDecay(7.0f)
	, HuntingLookAroundTimer(3.0f)
	, Walking(false)
	, Running(true)
	, StaminaIncrease(7.0f)
	, TargetPoint(0.0f, 0.0f, 0.0f)
	, RotationSpeed(11.0f)
	, LastPosition(0.0f)
	, HearingLocation(0.0f)
	, HeardSound(false)
	, HeardNoicesCount(6)
	, AwareTimer(0.0f)
	, InitialAwareTimerValue(240.0f)
	, PlayerIsSeen(false)
	, TimeSinceUpdatingRooms(0.0f)
	, SneakCalulation(false)
	, InvestigationCaluclation(false)
	, SoundLifeTime(0.0f)
	, IsPlayerInBasement(false)
	, CollidedDoor(nullptr)
	, DoorRotateBack(false)
	, IsDoorLocked(false)
	, InitialDoorRot(0,0,0)
	, Stunned(false)
	, StunnedDuration(2.0f)
	, MagdaSearchingTimer(100.0f)
	, RestlessRunning(false)
	, MovementSpeedIncreaseMultiplier(1.5)
	, MovementSpeedStartIncreaseMultiplier(0.7)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	FNeed Cravings(FString("Cravings"), 0.0f , 1.0f,2.0f, 3.0f,2.0f, 1);
	FNeed Energy(FString("Energy"), 0.0f,1.0f,4.0f, 1.0f, 3.0f, 2);
	FNeed Entertainment(FString("Entertainment"), 0.0f,1.0f,1.0f,3.0f,2.0f, 3);
	FNeed Bathroom(FString("Bathroom"), 0.0f ,1.0f,4.0f, 1.0f,2.0f, 4);
	FNeed Restless(FString("Restless"),0.0f,1.0f,1.0f,3.0f,2.0f, 0);
		
	Needs.Add(Cravings);
	Needs.Add(Energy);
	Needs.Add(Entertainment);
	Needs.Add(Bathroom);
	Needs.Add(Restless);
	
	Patroltimer = 1.0f;

	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BoxCollider"));
	CapsuleCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	CapsuleCollider->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	SetRootComponent(CapsuleCollider);

	mpFloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));

	DoorCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorCollision"));
	DoorCollider->SetupAttachment(CapsuleCollider);
	DoorCollider->SetRelativeLocation(FVector(0, 0, 70.0f));
	DoorCollider->SetEnableGravity(false);
	DoorCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	DoorCollider->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	DoorCollider->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	DoorCollider->SetBoxExtent(FVector(32, 60, 32));

	OverlapDoorSphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapDoorSphere"));
	OverlapDoorSphereCollider->SetRelativeLocation(FVector(0, 0, 70.0f));
	OverlapDoorSphereCollider->SetupAttachment(CapsuleCollider);
	OverlapDoorSphereCollider->SetEnableGravity(false);
	OverlapDoorSphereCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	OverlapDoorSphereCollider->SetCollisionResponseToChannel( ECC_WorldDynamic, ECR_Ignore );
	OverlapDoorSphereCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapDoorSphereCollider->SetSphereRadius(90.0f);

	mMoveRequest.SetUsePathfinding(true);
	mMoveRequest.SetProjectGoalLocation(true);
	mMoveRequest.SetAllowPartialPath(true);
	
	InitialHearingRange = mHearingRange;
	InitialHeardNoicesCount = HeardNoicesCount;

	ColliderForPlayerAttacks = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "ThrownItemsCollider" ) );
	ColliderForPlayerAttacks->SetupAttachment( CapsuleCollider );
	ColliderForPlayerAttacks->SetNotifyRigidBodyCollision( true );
	ColliderForPlayerAttacks->SetCollisionProfileName( "OverlapAll" );
	ColliderForPlayerAttacks->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	ColliderForPlayerAttacks->SetCollisionResponseToChannel( ECC_GameTraceChannel2, ECR_Block );
}

bool AGreta::IsPlayerInVisibleRange()
{
	FVector EnemyForwardDirection = GetActorForwardVector();
	FVector PlayerPosition = FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, GetActorLocation().Z);

	FVector EnemyToPlayer = PlayerPosition - GetActorLocation();

	double dot = EnemyToPlayer.DotProduct(EnemyForwardDirection, EnemyToPlayer);

	if (EnemyToPlayer.Length() <= VisibleRange && dot > 0)
	{

		FHitResult Hit;
		FCollisionObjectQueryParams params;
		params.AddObjectTypesToQuery(ECC_WorldStatic);
		if((Player->GetActorLocation().Z - GetActorLocation().Z) < 10.0f )
		GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), FVector(Player->GetActorLocation().X, Player->GetActorLocation().Y, GetActorLocation().Z), params);
		else
		{
			GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), Player->GetActorLocation(), params);
		}

		if(Hit.GetActor() == nullptr)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void AGreta::StateMachine(float DeltaTime)
{
	switch (CurrentState)
	{
	case State::PATROL:
		Patrol(DeltaTime);
		break;
	case State::INVESTIGATION:
		Investigating(DeltaTime);
		break;
	case State::HUNTING:
		Hunting(DeltaTime);
		break;
	case State::SNEAK:
		Sneaking(DeltaTime);
		break;
	}
}

void AGreta::Patrol(float DeltaTime)
{
	if (IsPlayerInVisibleRange())
	{
		CurrentState = State::HUNTING;
		OnBeginHunt();
	}
	if((mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal && Patroltimer <= 0) || mMoveRequestResult == EPathFollowingRequestResult::Failed)
	{
		if(Aware)
		{
			Patroltimer = FMath::RandRange(12, 18);

			if(!IsPlayerInBasement)
			{
				PatrolFloor(HouseRooms, DeltaTime);
			}
			else
			{
				PatrolFloor(BasementRooms, DeltaTime);
			}
		}
		else
		{
			Patroltimer = FMath::RandRange(12, 24);
			GetNewMovePoint();
		}
		
	}
	mMoveRequestResult = AIController->MoveTo(mMoveRequest);

	if (GetActorLocation() == LastPosition)
		return;

	LastPosition.Z = GetActorLocation().Z;
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPosition, GetActorLocation());
	Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
	SetActorRotation(Rotator);
	LastPosition = GetActorLocation();
}

void AGreta::Hunting(float DeltaTime)
{

	mMoveRequest.UpdateGoalLocation(TargetPoint);
	mMoveRequestResult = AIController->MoveTo(mMoveRequest);

	TargetPoint.Z = GetActorLocation().Z;
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPoint);
	Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, GetWorld()->GetDeltaSeconds(), RotationSpeed);
	SetActorRotation(Rotator);

	if (IsPlayerInVisibleRange())
	{
		TargetPoint = Player->GetActorLocation();
		Aware = true;
		if(Stamina > 30 && Running == false)
		{
			Running = true;
			Walking = false;
			mMovementSpeed *= MovementSpeedStartIncreaseMultiplier;
			mpFloatingPawnMovement->MaxSpeed = mMovementSpeed; 
		}
		else if(Stamina <= 0 && Walking == false )
		{
			Walking = true;
			Running = false;
		}
		if (Running)
		{
			mMovementSpeed = FMath::FInterpTo(mMovementSpeed, OrginalMovementSpeed * MovementSpeedIncreaseMultiplier, DeltaTime, 2.0f);
			
			if(mMovementSpeed > OrginalMovementSpeed * 1.5 - 1.0f)
				Stamina -= StaminaDecay * DeltaTime;

			mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;
		}
		else if (Walking || mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			mMovementSpeed = FMath::FInterpTo(mMovementSpeed, OrginalMovementSpeed, DeltaTime, 2.0f);
			if(mMovementSpeed < OrginalMovementSpeed + 1.0f)
			Stamina += StaminaIncrease * DeltaTime;

			mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;
		}
		
	}
	else if(mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		if((HuntingLookAroundTimer -= DeltaTime) <= 0 )
		{
			if(!IsPlayerInVisibleRange())
			{
				//TODO:: Heard a sound further than 3 meters
				if(HeardSound)
				{
					mMovementSpeed = OrginalMovementSpeed;
					mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;

					int rand = FMath::RandRange(1, 2);

					if(rand == 1)
					{
						OnEndHunt();
						CurrentState = State::SNEAK;
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Red, FString("Sneaking"));
						SneakCalulation = true;
					}
					else
					{
						OnEndHunt();
						CurrentState = State::INVESTIGATION;
						InvestigationCaluclation = true;
						GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Red, FString("Investigating"));
						SoundTimer = 0;
					}
					
				}
				else
				{
					OnEndHunt();
					mMovementSpeed = OrginalMovementSpeed;
					mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;
					CurrentState = State::INVESTIGATION;
					InvestigationCaluclation = true;
					GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Red, FString("Investigating"));
					SoundTimer = 0;
				}
				
				return;
			}
		}
	}
	else
	{
		HuntingLookAroundTimer = 3.0f;
	}
}

int AGreta::NeedCalculation()
{
	float T = 0;
	TArray<int> RandomizeDestination;
	for(int i = 0; i < Needs.Num(); i++)
	{
		if(Needs[i].RoomType != 0)
		T += (0.01f * FMath::Pow(Needs[i].NeedValue, 2));
		
	}
	if(T == 0)
	{
		int rand = 0;
		while(true)
		{
			rand = FMath::RandRange(0, (Needs.Num() - 1));
			if(Needs[rand].RoomType == 0)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		
		return Needs[rand].RoomType;
	}
	for(int j = 0; j < Needs.Num(); j++)
	{
		if (Needs[j].RoomType == 0)
			continue;
		int NeedsValueProcent = (Needs[j].NeedValue / T) * 100;
		for(int k = 0; k < NeedsValueProcent; k++)
		{
			RandomizeDestination.Add(Needs[j].RoomType);
		}
		int rand = FMath::RandRange(0,(RandomizeDestination.Num() - 1));
		if(rand < 0)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Red, "Needs Value was negative");
		}
		if(RandomizeDestination.Num() > j && RandomizeDestination.Num() > rand)
		RandomizeDestination.Swap(j, rand);
	}
	int RoomValue = RandomizeDestination[FMath::RandRange(0, (RandomizeDestination.Num() - 1))];
	for(int l = 0; l < Needs.Num(); l++)
	{
		if(Needs[l].RoomType == RoomValue)
		{
			return Needs[l].RoomType;
		}

	}
	return 0;
}

void AGreta::GetNewMovePoint()
{
	 
	
		if(!IsPlayerInBasement)
		{
			if(HouseRooms.Num() > 0)
			mMoveRequest.UpdateGoalLocation(SelectRoom(HouseRooms,NeedCalculation()));
		}
		else
		{
			if (BasementRooms.Num() > 0)
			mMoveRequest.UpdateGoalLocation(SelectRoom(BasementRooms, NeedCalculation()));
		}
		
}

// Called when the game starts or when spawned
void AGreta::BeginPlay()
{
	Super::BeginPlay();

	OrginalMovementSpeed = mMovementSpeed;

	mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoom::StaticClass(), FoundActors);


	for (int i = 0; i < FoundActors.Num(); i++)
	{
		ARoom* Temp = Cast<ARoom>(FoundActors[i]); 
		if(!Temp->GetIsDownStairs())
		HouseRooms.Add(Temp);
		else
		{
			BasementRooms.Add(Temp);
		}
	}

	if(HouseRooms.Num() > 0)
		mMoveRequest.SetGoalLocation(SelectRoom(HouseRooms,NeedCalculation()));
	else
	{
		mMoveRequest.SetGoalLocation(SelectRoom(BasementRooms, NeedCalculation()));
	}


	mpNavMesh = FNavigationSystem::GetCurrent< UNavigationSystemV1 >(this);

	AActor* Temp = UGameplayStatics::GetActorOfClass(GetWorld(), AVRPlayer::StaticClass());

	Player = Cast<AVRPlayer>(Temp);

	AIController = dynamic_cast<AGretaAiController*>(GetController());

	Magda = Cast<AMagda>(UGameplayStatics::GetActorOfClass(GetWorld(), AMagda::StaticClass()));

	if(Magda == nullptr || !Magda)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.0f, FColor::Red, "Magda isn't in the Level");
	}

	TimePerSoundCall = UKismetMathLibrary::RandomFloatInRange(MinTimePerSoundCall,MaxTimePerSoundCall);

	ColliderForPlayerAttacks->OnComponentHit.AddDynamic( this, &AGreta::NotifyPlayerAttack );

	InitialStunnedDuration = StunnedDuration;

	OverlapDoorSphereCollider->OnComponentBeginOverlap.AddDynamic( this, &AGreta::OnDoorColliderBeginOverlap );
}

void AGreta::NotifyPlayerAttack( UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector HitLocation, const FHitResult& Hit )
{
	Super::NotifyActorBeginOverlap(Other);
	if(Other->ActorHasTag("Axe"))
	{
		Player->OnKillGreta();
		GEngine->AddOnScreenDebugMessage(7, 1.0f, FColor::Blue, "YOU KILLED GRETA AJABAJA");
	}

	OnHitByPlayer();
	Stunned = true;
	

}

void AGreta::OnDoorColliderBeginOverlap( UPrimitiveComponent* OverlappedComponent
										 , AActor* OtherActor
										 , UPrimitiveComponent* OtherComp
										 , int32 OtherBodyIndex
										 , bool bFromSweep
										 , const FHitResult& SweepResult )
{
	Super::NotifyActorBeginOverlap( OtherActor );

	if ( Cast<AVRPlayer>( OtherActor ) == Player && IsPlayerInVisibleRange() )
	{
		OnAttack();	
		Player->Hit();
	}
}

FVector AGreta::SelectRoom(TArray<ARoom*> Rooms,int RoomType)
{
	TArray<FVector>RoomsByType;

	for(int i = 0; i < Rooms.Num(); i++)
	{
		if(Rooms[i]->RoomType == RoomType)
		{
			RoomsByType.Add(Rooms[i]->GetActorLocation());
		}
	}
	int Rand = FMath::RandRange(0, RoomsByType.Num() - 1);
	if(Rand >= RoomsByType.Num() || RoomsByType.IsEmpty())
	{
		Rooms[Rand];
	}
	return RoomsByType[Rand];
}

float AGreta::AddRoomPresence(ARoom* Room)
{
	if(Room == nullptr || Room->RoomType == 0)
	{
		return 0.0f;
	}

	for(int i = 0; i <	Needs.Num(); i++)
	{
		if(Room->RoomType == Needs[i].RoomType)
		{
			return Needs[i].NeedDecreaseRoomPresence;
		}
	}
	return 0.0f;
}

ARoom* AGreta::IsInRoom()
{
	for(int i = 0; i < HouseRooms.Num(); i++)
	{
		TArray<AActor*> OverlappingActors;
		HouseRooms[i]->BoxCollider->GetOverlappingActors(OverlappingActors);

		for(int k = 0; k < OverlappingActors.Num(); k++)
		{
			if(OverlappingActors[k] == this)
			{
				return HouseRooms[i];
			}
		}
	}
	for (int i = 0; i < BasementRooms.Num(); i++)
	{
		TArray<AActor*> OverlappingActors;
		BasementRooms[i]->BoxCollider->GetOverlappingActors(OverlappingActors);

		for (int k = 0; k < OverlappingActors.Num(); k++)
		{
			if (OverlappingActors[k] == this)
			{
				return BasementRooms[i];
			}
		}
	}
	return nullptr ;
}

void AGreta::Sneaking(float DeltaTime)
{
	if(IsPlayerInVisibleRange())
	{
		CurrentState = State::HUNTING;
		OnBeginHunt();
		Stamina = MaxStamina;
		Aware = true;
		return;
	}
	if(SneakCalulation)
	{
		if(!IsPlayerInBasement)
		{
			SneakCalculation(HouseRooms);
		}
		else
		{
			SneakCalculation(BasementRooms);
		}
		SneakCalulation = false;
	}
	mMoveRequestResult = AIController->MoveTo(mMoveRequest);

	if(mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		CurrentState = State::PATROL;
		SoundTimer = 0;
	}

}

void AGreta::Investigating(float DeltaTime)
{
	

	if (IsPlayerInVisibleRange())
	{
		CurrentState = State::HUNTING;
		OnBeginHunt();
		Stamina = MaxStamina;
		Aware = true;
		return;
	}
	if (mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal && InvestigationCaluclation)
	{
		if (!IsPlayerInBasement)
			InvestigationCalculation(HouseRooms);
		else
		{
			InvestigationCalculation(BasementRooms);
		}
		InvestigationCaluclation = false;
	}
	 mMoveRequestResult = AIController->MoveTo(mMoveRequest);

	 if(mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
	 {
		 if (!IsPlayerInVisibleRange())
		 {
			 CurrentState = State::PATROL;
		 	SoundTimer = 0;
		 }
	 }

	//TODO: 30% CHANCE TO LOOK INSIDE CLOSET 
}

void AGreta::Action()
{
	if(Aware)
	{
		if (MagdaSearchingTimer <= 0)
		{
			OnTellMagdaToSearch();
			Magda->StartSearching = true;
			MagdaSearchingTimer = InitialMagdaSearchingTimer;
		}
	}
	else
	{
		for(FNeed Need: Needs)
		{
			if(Need.NeedName == "Restless" && Need.NeedValue >= 90)
			{
				RestlessRunning = true;
				GetNewMovePoint();
				return;
			}
		}
	}
	
	
}

void AGreta::OffSetLocation()
{
	FHitResult Hit;
	FVector Offset(0, 0, 1000);
	FCollisionObjectQueryParams params;
	params.AddObjectTypesToQuery(ECC_WorldStatic);
	GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), GetActorLocation() - Offset, params);

	Offset.Z = CapsuleCollider->GetUnscaledCapsuleHalfHeight() + 10;
	if ((Hit.Location - GetActorLocation()).Length() < 100.0f)
		SetActorLocation(Hit.Location + Offset);
}

void AGreta::PatrolFloor(TArray<ARoom*> Rooms, float DeltaTime)
{
	if (Rooms.IsEmpty())
	{
		return;
	}
	ARoom* RoomToMoveToo = Rooms[0];
	ARoom* IsInAroom = IsInRoom();
	float RoomValue = (RoomToMoveToo->GetActorLocation() - GetActorLocation()).Length() * RoomToMoveToo->TimeSinceSeenGreta;
	for (int i = 1; i < Rooms.Num(); i++)
	{
		float Temp = (Rooms[i]->GetActorLocation() - GetActorLocation()).Length() * Rooms[i]->TimeSinceSeenGreta;
		if (Temp > RoomValue)
		{
			RoomValue = Temp;
			RoomToMoveToo = Rooms[i];
		}
		if (Rooms[i] != IsInAroom)
			Rooms[i]->TimeSinceSeenGreta += TimeSinceUpdatingRooms * DeltaTime;
	}

	mMoveRequest.UpdateGoalLocation(RoomToMoveToo->GetActorLocation());
}

void AGreta::SneakCalculation(TArray<ARoom*> Rooms)
{
	FVector MoveToLocation = HearingLocation;
	if (Rooms.Num() <= 0)
	{
		return;
	}
	ARoom* ActiveRoomNav = Rooms[0];
	float ShortestDistance = (Rooms[0]->GetActorLocation() - MoveToLocation).Length();
	for (int i = 1; i < Rooms.Num(); i++)
	{
		if ((Rooms[i]->GetActorLocation() - MoveToLocation).Length() < ShortestDistance)
		{
			ActiveRoomNav = Rooms[i];
			ShortestDistance = (Rooms[i]->GetActorLocation() - MoveToLocation).Length();
		}
	}
	if (!ActiveRoomNav->mNavMods.IsEmpty())
	{
		ShortestDistance = (ActiveRoomNav->mNavMods[0]->GetActorLocation() - MoveToLocation).Length();
		ANavMod* ClosestNavMod = ActiveRoomNav->mNavMods[0];
		for (int k = 0; k < ActiveRoomNav->mNavMods.Num(); k++)
		{
			if ((ActiveRoomNav->mNavMods[k]->GetActorLocation() - MoveToLocation).Length() < ShortestDistance)
			{
				ClosestNavMod = ActiveRoomNav->mNavMods[k];
				ShortestDistance = (ActiveRoomNav->mNavMods[k]->GetActorLocation() - MoveToLocation).Length();
			}
		}
		ClosestNavMod->IsLowCost = false;
		ClosestNavMod->ChangeCost();
	}
	mMoveRequest.UpdateGoalLocation(HearingLocation);
}

void AGreta::InvestigationCalculation(TArray<ARoom*> Rooms)
{
	FVector RoomVector;
	FVector EnemyToRoomVector;
	TArray<ARoom*> Temp;
	for (int i = 0; i < Rooms.Num(); i++)
	{
		RoomVector = FVector(Rooms[i]->GetActorLocation().X, Rooms[i]->GetActorLocation().Y, GetActorLocation().Z);
		EnemyToRoomVector = FVector(GetActorLocation() - Rooms[i]->GetActorLocation());

		float Angle = FMath::Acos(EnemyToRoomVector.DotProduct(GetActorForwardVector(), EnemyToRoomVector) / EnemyToRoomVector.Length() * GetActorForwardVector().Length());
		float Distance = (Rooms[i]->GetActorLocation() - GetActorLocation()).Length();
		Angle = FMath::RadiansToDegrees(Angle);
		if (Distance < 800.0f && Angle <= 90)
		{
			Temp.Add(Rooms[i]);
		}
	}
	if (!Temp.IsEmpty())
	{
		int rand = FMath::RandRange(0, Temp.Num() - 1);
		mMoveRequest.UpdateGoalLocation(Temp[rand]->GetActorLocation());
	}
	else
	{
		CurrentState = State::PATROL;
		SoundTimer = 0;
	}
}

void AGreta::OpenDoors(float DeltaTime)
{
	OverlapDoorSphereCollider->UpdateOverlaps();
	TArray<AActor*> FoundActors;
	OverlapDoorSphereCollider->GetOverlappingActors(FoundActors);
	bool RotateToOrginalRotation = true;
		for (AActor* Actor : FoundActors)
		{
			ADoorClass* Temp = Cast<ADoorClass>(Actor);
			if(Temp)
			{
				if (!CollidedDoor)
				{
					CollidedDoor = Temp;
					DoorRotateBack = true;
					RotateToOrginalRotation = false;
					GEngine->AddOnScreenDebugMessage(2, 3.0f, FColor::Blue, FString::SanitizeFloat(CollidedDoor->Door->GetRelativeRotation().Yaw));
					InitialDoorRot = CollidedDoor->Door->GetRelativeRotation();
					break;
				}
				else
				{
					if (Temp == CollidedDoor)
					{
						RotateToOrginalRotation = false;
						break;
					}
				}
			}
			
		}
	if(RotateToOrginalRotation && CollidedDoor && DoorRotateBack)
	{
		FRotator Rotator = FMath::RInterpConstantTo(CollidedDoor->Door->GetRelativeRotation(), InitialDoorRot, DeltaTime, 50.0f);
		CollidedDoor->Door->SetRelativeRotation(Rotator);
		float Dif = abs(CollidedDoor->Door->GetRelativeRotation().Yaw) - abs(InitialDoorRot.Yaw);
		GEngine->AddOnScreenDebugMessage(7, 1.0f, FColor::Green, FString::SanitizeFloat(Dif));
		if(CollidedDoor->Door->GetRelativeRotation().Yaw == InitialDoorRot.Yaw  || abs(Dif) < 1.5f)
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, "Rotation finished");
			CollidedDoor->Door->SetRelativeRotation(InitialDoorRot);
			if(IsDoorLocked)
			{
				IsDoorLocked = false;
				CollidedDoor->Lock();
			}
			CollidedDoor->Door->SetSimulatePhysics(false);
			CollidedDoor->Door->SetSimulatePhysics(true);
			CollidedDoor = nullptr;
			return;
		}
		
	}
	if(CollidedDoor && !RotateToOrginalRotation)
	{

		if (!ShouldOpenClosedDoors)
		{
			if (!CollidedDoor->bUnlocked)
			{
				NewRandomMoveTo();
				CollidedDoor = nullptr;
			}
			else
			{
				OnOpenDoor();
			}
			
		}
		else
		{
			if (!CollidedDoor->bUnlocked)
			{
				IsDoorLocked = true;
				CollidedDoor->Unlock();
			}
			OnOpenDoor();
			
		}
	}
	
}

void AGreta::NewRandomMoveTo()
{
	if(!IsPlayerInBasement && BasementRooms.Num() > 0)
	{
		mMoveRequest.UpdateGoalLocation(BasementRooms[FMath::RandRange(0, BasementRooms.Num() -1)]->GetActorLocation());
		mMoveRequestResult = AIController->MoveTo(mMoveRequest);
	}
	else
	{
		if(HouseRooms.Num() > 0)
		{
			mMoveRequest.UpdateGoalLocation(HouseRooms[FMath::RandRange(0, HouseRooms.Num() -1)]->GetActorLocation());
			mMoveRequestResult = AIController->MoveTo(mMoveRequest);
		}
	}
}

// Called every frame
void AGreta::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OpenDoors(DeltaTime);
	if(Player->GetActorLocation().Z < 18600.0f)
	{
		IsPlayerInBasement = true;
	}
	else
	{
		IsPlayerInBasement = false;
	}

	if (IsPlayerInVisibleRange())
	{
		AwareTimer = InitialAwareTimerValue;
		if ((VisibleTimer -= DeltaTime) < 0.0f)
		{
			Aware = true;
			mHearingRange = InitialHearingRange * 1.3f;	
			AIController->ChangeHearingRange(mHearingRange);

		}
	}
	else if(HeardNoicesCount <= 0)
	{
		Aware = true;
	}
	else if(FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= 500.0f && Player->IsSprinting())
	{
		Aware = true;
	}
	else
	{
		VisibleTimer = 0.6f;
	}

	if(Aware)
	{
		if (!IsPlayerInVisibleRange())
		{
			TimeSinceUpdatingRooms += DeltaTime;
			if ((AwareTimer -= DeltaTime) <= 0)
			{
				Aware = false;
				HeardNoicesCount = InitialHeardNoicesCount;
				mHearingRange = InitialHearingRange;

				for(int k = 0; k < HouseRooms.Num(); k++)
				{
					HouseRooms[k]->TimeSinceSeenGreta = 0;
				}
				for(int u = 0; u < BasementRooms.Num(); u++)
				{
					BasementRooms[u]->TimeSinceSeenGreta = 0;
				}
			}
		}
	}
	else
	{
		ARoom* RoomValueTemp = IsInRoom();
		for (int i = 0; i < Needs.Num(); i++)
		{

			if (Needs[i].RoomType != 0)
			{
				if (RoomValueTemp && RoomValueTemp->RoomType == Needs[i].RoomType)
				{
					if ((Needs[i].NeedDecreaseTimer -= DeltaTime) <= 0 && (Needs[i].NeedValue - Needs[i].NeedDecreaseRoomPresence) > 0)
					{
						Needs[i].NeedValue -= Needs[i].NeedDecreaseRoomPresence;
						Needs[i].NeedDecreaseTimer = Needs[i].OrigianlDecreaseTimer;

					}
				}
				else
				{
					if ((Needs[i].NeedIncreaseTimer -= DeltaTime) <= 0)
					{
						Needs[i].NeedValue += Needs[i].NeedIncreaseRate;
						Needs[i].NeedIncreaseTimer = Needs[i].OriginalIncreaseTimer;

					}
				}
			}
			else
			{
				if (RestlessRunning)
				{
					if (mMovementSpeed == OrginalMovementSpeed)
					{
						mMovementSpeed *= MovementSpeedIncreaseMultiplier;
					}
					mMoveRequestResult = AIController->MoveTo(mMoveRequest);
					if (mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
					{
						RestlessRunning = false;

					}
					Needs[i].NeedValue -= 6 * DeltaTime;
				}
				else
				{
					if (mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
					{
						if ((Needs[i].NeedIncreaseTimer -= DeltaTime) <= 0)
						{
							Needs[i].NeedValue += Needs[i].NeedIncreaseRate;
							Needs[i].NeedIncreaseTimer = Needs[i].OriginalIncreaseTimer;

						}
					}
					else
					{
						if ((Needs[i].NeedDecreaseTimer -= DeltaTime) <= 0 && (Needs[i].NeedValue - Needs[i].NeedDecreaseRoomPresence) > 0)
						{
							Needs[i].NeedValue -= Needs[i].NeedDecreaseRoomPresence;
							Needs[i].NeedDecreaseTimer = Needs[i].OrigianlDecreaseTimer;

						}
					}
				}
				
			}
		}
	}
	if (mMoveRequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal || CurrentState != State::PATROL)
	{
		Patroltimer -= DeltaTime;
	}
	if ((SoundLifeTime -= DeltaTime) <= 0)
	{
		HeardSound = false;
	}
	if(CurrentState != State::HUNTING)
	{
		Action();
	}
	if(AIController && AIController->GetHeardSound())
	{
		OnHeardSound();
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.0f, FColor::Blue, FString("Sound Sensed"));
		SoundLifeTime = HeardSoundLifeTime;
		HeardSound = true;
		HearingLocation = Player->GetActorLocation();
		HeardNoicesCount--;
		AIController->SetHeardSound(false);
	}
	OffSetLocation();
	if (!Stunned)
	{
		if (Magda != nullptr)
		{
			if (!Magda->IsBarking)
			{
				StateMachine(DeltaTime);
			}
			else
			{
				mMovementSpeed = OrginalMovementSpeed;
				mpFloatingPawnMovement->MaxSpeed = mMovementSpeed;
				if (IsPlayerInVisibleRange())
				{
					CurrentState = State::HUNTING;
					OnBeginHunt();
					Magda->IsBarking = false;
					return;
				}
				mMoveRequest.UpdateGoalLocation(Magda->GetActorLocation());
				mMoveRequestResult = AIController->MoveTo(mMoveRequest);

				if (mMoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
				{
					Magda->IsBarking = false;
				}
				if (GetActorLocation() == LastPosition)
					return;

				LastPosition.Z = GetActorLocation().Z;
				FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPosition, GetActorLocation());
				Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
				SetActorRotation(Rotator);
				LastPosition = GetActorLocation();
			}
		}
	}
	else
	{
		AIController->StopMovement();
		if((StunnedDuration -= DeltaTime) <= 0)
		{
			mMoveRequest.UpdateGoalLocation(GetActorLocation());
			mMoveRequestResult = AIController->MoveTo(mMoveRequest);
			if(CurrentState == State::PATROL)
			{
				Patroltimer = 0.0f;
			}
			Stunned = false;
			StunnedDuration = InitialStunnedDuration;
		}
	}
	MagdaSearchingTimer -= DeltaTime;

}


