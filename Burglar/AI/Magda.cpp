// Fill out your copyright notice in the Description page of Project Settings.


#include "Magda.h"
#include "../Player/VRPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMagda::AMagda()
	:MoveRequest(FAIMoveRequest())
	,MoveRequestResult()
	,SoundTimer(0)
	,TimePerSoundCall(.5f)
	,MinTimePerSoundCall(.5f)
	,MaxTimePerSoundCall(1)
	,Awake(false)
	,AwakeDuration(60.0f)
	,InitialAwakeDuration(0.0f)
	,MovementSpeed(100.0f)
	,InitialMovementSpeed(0.0f)
	,AwakeRange(70.0f)
	,CurrentState(MagdaState::Sleeping)
	,VisibleRange(800.0f)
	,BarkDuration(3.0f)
	,SearchRadius(300.0f)
	,PassiveRadius(900.0f)
	,IntialBarkDuration(0.0f)
	,StartSearching(false)
	,SearchIdleDuration(3.0f)
	,SearchDuration(30.0f)
	,IncreaseMovementSpeedMultiplier(2.0f)
	,StartBarkingDuration(3.0f)
	,ShouldStartBarking(false)
	,PassiveIdleDuration(0.0f)
	,PassiveIdleDurationMaxValue(8)
	,PassiveIdleDurationMinimumValue(2)
	,IsBarking(false)
	,RotationSpeed(15.0f)
	,LastPos(0.0f)
	,TargetPoint(0.0f)
	,HasReceivedBone(false)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collider"));
	SetRootComponent(CapsuleCollider);


	SkeletalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SkeletalMesh->SetupAttachment(CapsuleCollider);

	BoneCollider = CreateDefaultSubobject<USphereComponent>(TEXT("BoneCollider"));
	BoneCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoneCollider->SetupAttachment(CapsuleCollider);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovemnent"));


	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetProjectGoalLocation(true);
	MoveRequest.SetAllowPartialPath(true);
}

// Called when the game starts or when spawned
void AMagda::BeginPlay()
{
	Super::BeginPlay();

	InitialAwakeDuration = AwakeDuration;
	InitialMovementSpeed = MovementSpeed;
	IntialBarkDuration = BarkDuration;
	InitialSearchIdleDuration = SearchIdleDuration;
	InitialSearchDuration = SearchDuration;
	InitialStartBarkingDuration = StartBarkingDuration;


	FloatingPawnMovement->MaxSpeed = MovementSpeed;
	PassiveIdleDuration = FMath::RandRange(PassiveIdleDurationMinimumValue, PassiveIdleDurationMaxValue);


	PlayerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVRPlayer::StaticClass());
	AIController = dynamic_cast<AGretaAiController*>(GetController());

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "SleepPos", SleepingPositions);

	if(SleepingPositions.IsEmpty())
	{
		SleepingPositions.Add(this);
	}
	
	NavMesh = FNavigationSystem::GetCurrent< UNavigationSystemV1 >(this);

	 MoveRequest.SetGoalLocation(GetActorLocation());

	TimePerSoundCall = UKismetMathLibrary::RandomFloatInRange(MinTimePerSoundCall,MaxTimePerSoundCall);

	
}

void AMagda::StateMachine(float DeltaTime)
{
	switch (CurrentState)
	{
	case MagdaState::Sleeping:
		Sleeping(DeltaTime);
		break;
	case MagdaState::Passive:
		Passive(DeltaTime);
		break;
	case MagdaState::Bark:
		Bark(DeltaTime);
		break;
	case MagdaState::Search:
		Search(DeltaTime);
		break;
	}
}

void AMagda::Sleeping(float DeltaTime)
{
	if(MoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal && !AlreadyOnSleepingPos())
	{
		int RandomSleepPos = FMath::RandRange(0, SleepingPositions.Num() - 1);
		MoveRequest.UpdateGoalLocation(SleepingPositions[RandomSleepPos]->GetActorLocation());
	}
	if((PlayerActor->GetActorLocation() - GetActorLocation()).Length() < AwakeRange)
	{
		Awake = true;
		CurrentState = MagdaState::Bark;
		IsBarking = true;
		SoundTimer = 0;
	}
	MoveRequestResult = AIController->MoveTo(MoveRequest);
	if (GetActorLocation() == LastPos)
		return;

	LastPos.Z = GetActorLocation().Z;
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPos, GetActorLocation());
	Rotator.Yaw -= 180.0f;
	Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
	SetActorRotation(Rotator);
	LastPos = GetActorLocation();
}

void AMagda::Passive(float DeltaTime)
{
	MoveRequestResult = AIController->MoveTo(MoveRequest);

	if(IsPlayerInVisibleRange(VisibleRange, false))
	{
		CurrentState = MagdaState::Bark;
		IsBarking = true;
		SoundTimer = 0;
	}

	if(MoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		
		if((PassiveIdleDuration -= DeltaTime ) <= 0)
		{
			FNavLocation NewMovePoint;
			NavMesh->GetRandomPointInNavigableRadius(GetActorLocation(), PassiveRadius, NewMovePoint);
			MoveRequest.UpdateGoalLocation(NewMovePoint);
			PassiveIdleDuration = FMath::RandRange(PassiveIdleDurationMinimumValue, PassiveIdleDurationMaxValue);
		}
	}
	
	if (GetActorLocation() == LastPos)
		return;

	LastPos.Z = GetActorLocation().Z;
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPos, GetActorLocation());
	Rotator.Yaw -= 180.0f;
	Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
	SetActorRotation(Rotator);
	LastPos = GetActorLocation();
	
}

void AMagda::Bark(float DeltaTime)
{
	MoveRequest.UpdateGoalLocation(GetActorLocation());
	if(!IsPlayerInVisibleRange(VisibleRange, false))
	{
		if((BarkDuration -= DeltaTime) <= 0)
		{
			BarkDuration = IntialBarkDuration;
			CurrentState = MagdaState::Passive;
		}
	}
	else
	{
		FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(TargetPoint, GetActorLocation());
		Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
		SetActorRotation(Rotator);

	}
	MoveRequestResult = AIController->MoveTo(MoveRequest);
	TargetPoint = PlayerActor->GetActorLocation();
}

void AMagda::Search(float DeltaTime)
{

	if((SearchDuration -= DeltaTime) <= 0)
	{
		AIController->StopMovement();
		CurrentState = MagdaState::Passive;
		MovementSpeed = InitialMovementSpeed;
		StartBarkingDuration = InitialStartBarkingDuration;
		PassiveIdleDuration = FMath::RandRange(PassiveIdleDurationMinimumValue, PassiveIdleDurationMaxValue);
		SearchDuration = InitialSearchDuration;
		MoveRequest.UpdateGoalLocation(GetActorLocation());
		FloatingPawnMovement->MaxSpeed = MovementSpeed;
		return;
	}
	if(IsPlayerInVisibleRange(SearchRadius,true))
	{
		ShouldStartBarking = true;
	}
	if(ShouldStartBarking)
	{
		if((StartBarkingDuration -= DeltaTime) <= 0)
		{
			SearchDuration = InitialSearchDuration;
			StartBarkingDuration = InitialStartBarkingDuration;
			CurrentState = MagdaState::Bark;
			IsBarking = true;
			SoundTimer = 0;
		}
	}

	if(MovementSpeed == InitialMovementSpeed)
	{
		MovementSpeed *= IncreaseMovementSpeedMultiplier;
		FloatingPawnMovement->MaxSpeed = MovementSpeed;
		
	}
	if(MoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		if((SearchIdleDuration -= DeltaTime) <= 0)
		{
			FNavLocation NewMoveLocation;
			NavMesh->GetRandomPoint(NewMoveLocation);
			MoveRequest.UpdateGoalLocation(NewMoveLocation.Location);
		}
	}
	MoveRequestResult = AIController->MoveTo(MoveRequest);

	if (GetActorLocation() == LastPos)
		return;

	LastPos.Z = GetActorLocation().Z;
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPos, GetActorLocation());
	Rotator.Yaw -= 180.0f;
	Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
	SetActorRotation(Rotator);
	LastPos = GetActorLocation();
}

bool AMagda::IsPlayerInVisibleRange(float Range, bool ThroughWalls)
{
	FVector EnemyForwardDirection = GetActorForwardVector();
	FVector PlayerPosition = FVector(PlayerActor->GetActorLocation().X, PlayerActor->GetActorLocation().Y, GetActorLocation().Z);

	FVector EnemyToPlayer = PlayerPosition - GetActorLocation();

	double dot = EnemyToPlayer.DotProduct(EnemyForwardDirection, EnemyToPlayer);

	
	
	if (EnemyToPlayer.Length() <= Range)
	{
		if (!ThroughWalls && dot > 0)
		{

			FHitResult Hit;
			FCollisionObjectQueryParams params;
			params.AddObjectTypesToQuery(ECC_WorldStatic);
			if ((PlayerActor->GetActorLocation().Z - GetActorLocation().Z) < 10.0f)
				GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), FVector(PlayerActor->GetActorLocation().X, PlayerActor->GetActorLocation().Y, GetActorLocation().Z), params);
			else
			{
				GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), PlayerActor->GetActorLocation(), params);
			}

			if (Hit.GetActor() == nullptr)
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
			if(PlayerActor->GetActorLocation().Z - GetActorLocation().Z <= 150.0f)
			return true;
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

void AMagda::IsOnGround()
{
	FHitResult Hit;
	FVector Offset(0, 0, 1000);
	FCollisionObjectQueryParams params;
	params.AddObjectTypesToQuery(ECC_WorldStatic);
	GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), GetActorLocation() - Offset, params);

	Offset.Z = CapsuleCollider->GetUnscaledCapsuleHalfHeight();
	if ((Hit.Location - GetActorLocation()).Length() < 100.0f)
		SetActorLocation(Hit.Location + Offset);
}

bool AMagda::AlreadyOnSleepingPos()
{
	for(int i = 0; i < SleepingPositions.Num(); i++)
	{
		if((SleepingPositions[i]->GetActorLocation() - GetActorLocation()).Length() < 30.0f)
		{
			return true;
		}
	}
	return false;
}

// Called every frame
void AMagda::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor*> FoundActors;
	BoneCollider->UpdateOverlaps();
	BoneCollider->GetOverlappingActors(FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (Actor->ActorHasTag("Bone"))
		{
			GEngine->AddOnScreenDebugMessage(9, 3.0f, FColor::Orange, "Dog Has THE BONE");
			HasReceivedBone = true;
		}
	}

	if (!HasReceivedBone)
	{


		if (Awake)
		{
			if ((AwakeDuration -= DeltaTime) <= 0)
			{
				if (CurrentState == MagdaState::Search)
				{
					MovementSpeed = InitialMovementSpeed;
					FloatingPawnMovement->MaxSpeed = MovementSpeed;
				}
				CurrentState = MagdaState::Sleeping;
				AwakeDuration = InitialAwakeDuration;
				Awake = false;
			}
		}
		if (StartSearching)
		{
			CurrentState = MagdaState::Search;
			SearchDuration = InitialSearchDuration;
			StartSearching = false;
		}


		StateMachine(DeltaTime);

		IsOnGround();

		SoundTimer += DeltaTime;
		if (SoundTimer > TimePerSoundCall)
		{
			SoundTimer = 0;
			TimePerSoundCall = UKismetMathLibrary::RandomFloatInRange(MinTimePerSoundCall, MaxTimePerSoundCall);

			if (CurrentState == MagdaState::Bark)
				WhileBarking();
		}
	}
	else
	{
		WhileHoldingBone();
		if (MoveRequestResult == EPathFollowingRequestResult::AlreadyAtGoal && !AlreadyOnSleepingPos())
		{
			int RandomSleepPos = FMath::RandRange(0, SleepingPositions.Num() - 1);
			MoveRequest.UpdateGoalLocation(SleepingPositions[RandomSleepPos]->GetActorLocation());
		}
		MoveRequestResult = AIController->MoveTo(MoveRequest);
		if (GetActorLocation() == LastPos)
			return;

		LastPos.Z = GetActorLocation().Z;
		FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(LastPos, GetActorLocation());
		Rotator.Yaw -= 180.0f;
		Rotator = FMath::RInterpTo(GetActorRotation(), Rotator, DeltaTime, RotationSpeed);
		SetActorRotation(Rotator);
		LastPos = GetActorLocation();
	}

}

