// Fill out your copyright notice in the Description page of Project Settings.


#include "SafePuzzleSystem.h"
#include "Burglar/Player/VRPlayer.h"
#include "GameFramework/Character.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ASafePuzzleSystem::ASafePuzzleSystem()
	: UnlockFeedback( nullptr )
	, CurrentSequenceNumber(0)
	, CurrentNumberRot(0.0f)
	, CurrentNumberOn(10.0f)
	, StopRotatingDuration(0.5f)
	, LastRotation(0.0f)
	, DoorLocked(true)
	, BeginOpeningSafe(false)
	, SafeDoorRotationSpeed(5.0f)
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Safe Mesh"));
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->SetupAttachment(StaticMesh);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASafePuzzleSystem::BeginPlay()
{
	Super::BeginPlay();

	
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "DoorKnob", FoundActors);

	DoorKnobActor = FoundActors[0];
	CurrentNumber = NumberSequence[CurrentSequenceNumber];
	
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "SafeDoor", FoundActors);

	UActorComponent* Temp;
	Temp = FoundActors[0]->GetComponentByClass(USafeDoorComponent::StaticClass());

	DoorComponent = Cast<USafeDoorComponent>(Temp);

	InitialStopRotatingDuration = StopRotatingDuration;

	
}

// Called every frame
void ASafePuzzleSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(DoorKnobActor->GetActorRotation() == LastRotation && DoorLocked == true)
	{
		if((StopRotatingDuration -= DeltaTime) <= 0)
		{
			if(IsItCorrectlyRotated() && (CurrentSequenceNumber < NumberSequence.Num()))
			{
				OnRightNumber();
				if ((CurrentSequenceNumber += 1 ) > NumberSequence.Num() - 1  )
				{
					OnUnlock();
					DoorLocked = false;
					BeginOpeningSafe = true;
					LastRotation = FRotator(0.0f);

					AVRPlayer* Player = Cast<AVRPlayer>(  GetWorld()->GetFirstPlayerController()->GetPawn() );
					if( Player && UnlockFeedback)
					{
						Player->PlayHapticEffect( UnlockFeedback, EControllerHand::Left );
						Player->PlayHapticEffect( UnlockFeedback, EControllerHand::Right );
					}
					return;
				}
				
				CurrentNumber = NumberSequence[CurrentSequenceNumber];
			}
			StopRotatingDuration = InitialStopRotatingDuration;
		}

		
	}
	else
	{
		StopRotatingDuration = InitialStopRotatingDuration;
	}
	LastRotation = DoorKnobActor->GetActorRotation();

	if(BeginOpeningSafe)
	{
		OpenSafe(DeltaTime);
		if((DoorComponent->GetOwner()->GetRootComponent()->GetComponentRotation().Yaw - DoorComponent->GetTargetSafeRotation().Yaw) <= 0.5f)
		{
			BeginOpeningSafe = false;
			DoorComponent->UnLocked();
		}
	}
}

bool ASafePuzzleSystem::IsItCorrectlyRotated()
{
	if(CurrentNumberOn == CurrentNumber)
	{
		return true;
	}
	return false;
}

void ASafePuzzleSystem::AddRotationNumber(int Dir)
{
	CurrentNumberRot += Dir;
	OnRotationTick();
	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Orange, FString::FromInt(CurrentNumberRot));

	if(CurrentNumberRot == -7)
	{
		if(CurrentNumberOn == 0)
		{
			CurrentNumberOn = 90;
		}
		else
		{
			CurrentNumberOn -= 10;
		}
		
		GEngine->AddOnScreenDebugMessage(2, 3.0f, FColor::Green, FString::FromInt(CurrentNumberOn));
		CurrentNumberRot = 0;
	}
	else if (CurrentNumberRot == 7)
	{
		if (CurrentNumberOn == 90)
		{
			CurrentNumberOn = 0;
		}
		else
		{
			CurrentNumberOn += 10;
		}
		GEngine->AddOnScreenDebugMessage(2, 3.0f, FColor::Green, FString::FromInt(CurrentNumberOn));
		CurrentNumberRot = 0;
	}
}

void ASafePuzzleSystem::OpenSafe(float DeltaTime)
{
	FRotator Temp = FMath::RInterpTo(DoorComponent->GetOwner()->GetRootComponent()->GetComponentRotation(), DoorComponent->GetTargetSafeRotation(), DeltaTime, SafeDoorRotationSpeed);
	DoorComponent->GetOwner()->GetRootComponent()->SetWorldRotation(Temp);
}

