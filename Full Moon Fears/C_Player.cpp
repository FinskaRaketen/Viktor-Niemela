// Fill out your copyright notice in the Description page of Project Settings.
#include "game/Player/C_Player.h"
#include "UnrealClient.h"
#include "SceneView.h"
#include <DrawDebugHelpers.h>
#include <math.h>
#include <AIController.h>
#include <GameFramework/FloatingPawnMovement.h>
#include <Kismet/KismetMathLibrary.h>
#include <Math/UnrealMathVectorCommon.h>
#include <game/Interactable/Snow/C_SnowInteractable.h>
#include <game/Interactable/Tree/C_InteractableTree.h>
#include "Kismet/GameplayStatics.h"
#include <game/Player/C_GoalIndicator_Widget.h>
#include <Blueprint/UserWidget.h>
#include <game/GameStateManager/Game/C_Goal.h>
#include "Math/Quat.h"
#include <game/Player/C_MoonstoneUI.h>
#include "game/Enemy/C_Enemy.h"
#include <NiagaraFunctionLibrary.h>
#include <NiagaraComponent.h>
#include <NiagaraSystem.h>
#include "game/Animation/C_AnimInstance.h"

// Sets default values
AC_Player::AC_Player()
	: m_pWorld( nullptr )
	, m_pCamera( nullptr )
	, m_pAIController( nullptr )
	, m_pFloatingPawnMovement( nullptr )
	, m_MovementSpeed( 1000 )
	, m_pCurrentInteractable( nullptr )
	, m_CameraDistance( 10.0, 10.0, 0.0 )
	, m_RotationDirection( 1, 0, 0 )
	, m_LookAtPosition( 0 )
	, m_MovementDirection( 0 )
	, m_RotationSpeed( 2 )
	, m_DistanceToInteractable( 0 )
	, m_OnFire( false )
	, m_FireTimer( 0 )
	, m_FireDamageTimer( 0 )
	, m_FireSoundTimer( 0 )
	, m_FireSoundDuration( 1.528250f )
	, m_TimeBetweenFootsteps( .5f )
	, m_FootstepTimer( 0 )
	, m_ChasedSoundDuration( 5 )
	, m_ChasedSoundTimer( 0 )
	, m_NotChasedSoundDuration( 5 )
	, m_NotChasedSoundTimer( 0 )
	, m_TimeBetweenFireDamage( 5 )
	, m_pFireEffect( nullptr )
	, m_FireEffectComp( nullptr )
	, m_FireIntensity( 50 )
	, m_TimeUntilFireIsOut( 20 )
	, m_ClimbHeight( 200 )
	, m_CurrentState( EPlayerStates::ES_IDLE )
	, m_MaxHealth( 5 )
	, m_CurrentHealth( 5 )
	, m_MoonstoneCharges( 0 )
	, m_MaxMoonstoneCharges( 5 )
	, m_HowlTimer( 0 )
	, m_MaxTimeHowl( 10 )
	, m_Hiding( false )
	, m_Dead( false )
	, m_pClimbingUpAnimation( nullptr )
	, m_pClimbingDownAnimation( nullptr )
	, m_pIdleAnimation( nullptr )
	, m_pRunAnimation( nullptr )
	, m_pDieAnimation( nullptr )
	, m_pHowlingAnimation( nullptr )
	, m_GoalIndicatorClass( nullptr )
	, m_GoalPos( 0, 0, 0 )
	, m_pGoalIndicator( nullptr )
	, m_MoonstoneUIClass( nullptr )
	, m_pMoonstoneUI( nullptr )
	, m_CallOnEnemyRadius( 10000 )
	, m_pEnemyClass( nullptr )
	, m_interactingSnow( false )
	, m_OnIce( false )
	, m_pAnimInstance( nullptr )
	, m_AmountOfPos( 8 )
	, m_MaxAmountOfPos( 8 )
	, m_InnerRadius( 100 )
	, m_OuterRadius( 150 )
	, m_JumpSpeed( 6.0f )
	, m_IsJumping( false )
	, m_tmpMovementDirection( 0, 0, 0 )
	, m_InteractButtonCheck(false)
	, m_IsChased(false)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pCapsuleCollider = CreateAbstractDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollider" ) );
	SetRootComponent( m_pCapsuleCollider );

	m_pPlayerMesh = CreateAbstractDefaultSubobject<USkeletalMeshComponent>( TEXT( "PlayerMesh" ) );
	m_pPlayerMesh->SetupAttachment( m_pCapsuleCollider );

	m_pInteractionCollider = CreateAbstractDefaultSubobject<UCapsuleComponent>( TEXT( "InteractionCollider" ) );
	m_pInteractionCollider->SetupAttachment( m_pCapsuleCollider );

	m_pFloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>( TEXT( "FloatingPawnMovement" ) );
	m_pFloatingPawnMovement->MaxSpeed = m_MovementSpeed;

	ConstructorHelpers::FObjectFinder<UClass> CameraClass( TEXT( "Class'/Script/game.C_Camera'" ) );
	if( CameraClass.Object )
		m_pCameraClass = CameraClass.Object;

	ConstructorHelpers::FObjectFinder<UClass> EnemyClass( TEXT( "Class'/Script/game.C_Enemy'" ) );
	if( EnemyClass.Object )
		m_pEnemyClass = EnemyClass.Object;
}

// Called when the game starts or when spawned
void AC_Player::BeginPlay()
{
	Super::BeginPlay();
	m_CurrentHealth = m_MaxHealth;
	m_OriginalJumpSpeed = m_JumpSpeed;
	m_JumpSpeed = -1.0f;
	TArray<AActor*> actors;

	m_pWorld = GetWorld();
	UGameplayStatics::GetAllActorsOfClass( m_pWorld, m_pCameraClass, actors );

	for( int i = 0; i < actors.Num(); i++ )
	{
		AC_Camera* test = Cast<AC_Camera>( actors[i] );
		if( test )
		{
			m_pCamera = test;
		}
	}

	m_pAIController = Cast<AAIController>( GetController() );

	FVector Forward = m_pCamera->GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();
	m_LookAtPosition = GetActorLocation() + ( m_pCamera->GetActorRightVector() * m_RotationDirection.Y ) + ( Forward * m_RotationDirection.X );

	if( m_GoalIndicatorClass )
	{
		m_pGoalIndicator = CreateWidget<UC_GoalIndicator_Widget>( m_pWorld, m_GoalIndicatorClass );
		m_pGoalIndicator->AddToPlayerScreen();
		m_pGoalIndicator->ChangeVisibility( false );
	}

	if( m_MoonstoneUIClass )
	{
		m_pMoonstoneUI = CreateWidget<UC_MoonstoneUI>( m_pWorld, m_MoonstoneUIClass );
		m_pMoonstoneUI->AddToPlayerScreen();
		m_pMoonstoneUI->ChangeMoonstoneCharges( m_MoonstoneCharges );

		m_pMoonstoneUI->SetMaxHealth( m_MaxHealth );
		m_pMoonstoneUI->SetDamageIndicator( m_CurrentHealth );
	}

	m_PlayerStartPos = m_pPlayerMesh->GetRelativeLocation();

	m_pAnimInstance = Cast<UC_AnimInstance>( m_pPlayerMesh->GetAnimInstance() );
	m_pAnimInstance->ChangeAnimation( m_pIdleAnimation );

	if( m_AmountOfPos >= m_MaxAmountOfPos )
	{
		m_AmountOfPos = m_MaxAmountOfPos;
	}

	for( int i = 0; i < m_AmountOfPos; i++ )
	{
		float angle = ( 360 / m_AmountOfPos ) * i;

		// Inner circle pos
		FEnemyPosition TmpStructInner;

		float X = cos( FMath::DegreesToRadians( angle ) ) * m_InnerRadius/* + GetActorLocation().X*/;
		float Y = sin( FMath::DegreesToRadians( angle ) ) * m_InnerRadius /*+ GetActorLocation().Y*/;

		TmpStructInner.m_Pos.X = X;
		TmpStructInner.m_Pos.Y = Y;
		TmpStructInner.m_Pos.Z = 0;

		TmpStructInner.m_Occupied = false;

		m_pEnemyPositions.Add( TmpStructInner );
	}

	for( int i = 0; i < m_AmountOfPos; i++ )
	{
		float angle = ( 360 / m_AmountOfPos ) * i + ( ( 360 / m_AmountOfPos ) / 2 );

		// Outer circle pos
		FEnemyPosition TmpStructOuter;

		float X = cos( FMath::DegreesToRadians( angle ) ) * m_OuterRadius /*+ GetActorLocation().X*/;
		float Y = sin( FMath::DegreesToRadians( angle ) ) * m_OuterRadius /*+ GetActorLocation().Y*/;

		TmpStructOuter.m_Pos.X = X;
		TmpStructOuter.m_Pos.Y = Y;
		TmpStructOuter.m_Pos.Z = 0;

		TmpStructOuter.m_Occupied = false;

		m_pEnemyPositions.Add( TmpStructOuter );
	}
}

void AC_Player::StateMachine( float DeltaTime )
{
	switch( m_CurrentState )
	{
		case EPlayerStates::ES_IDLE:
			Idle( DeltaTime );
			break;

		case EPlayerStates::ES_MOVING:
			Moving( DeltaTime );
			break;

		case EPlayerStates::ES_CLIMBING_UP:
			ClimbUp( DeltaTime );
			break;

		case EPlayerStates::ES_HIDING:
			Hiding( DeltaTime );
			break;

		case EPlayerStates::ES_CLIMBING_DOWN:
			ClimbDown( DeltaTime );
			break;

		case EPlayerStates::ES_DEAD:
			Dead( DeltaTime );
			break;

		case EPlayerStates::ES_TRAPPED:
			Trapped( DeltaTime );
			break;

		case EPlayerStates::ES_HOWLING:
			Howling( DeltaTime );
			break;

		case EPlayerStates::ES_SNOW:
			InteractSnow( DeltaTime );
			break;

		case EPlayerStates::ES_UNHIDE:
			UnHide( DeltaTime );
			break;

		case EPlayerStates::ES_JUMP:
			Jump( DeltaTime );
			break;
	}
}

void AC_Player::Idle( float DeltaTime )
{
	bool GotInput = GetInput( DeltaTime );

	if( !m_pAnimInstance->IsPlaying( m_pIdleAnimation ) || !m_pAnimInstance->IsPlaying() )
	{
		m_pAnimInstance->ChangeAnimation( m_pIdleAnimation );
	}

	if( GotInput && !m_IsJumping )
	{
		m_CurrentState = EPlayerStates::ES_MOVING;
	}
	else if( m_IsJumping )
	{
		m_CurrentState = EPlayerStates::ES_JUMP;
	}

	if( m_pCurrentInteractable != nullptr && m_DistanceToInteractable < 1500 && IsPlayerInteracting )
	{
		m_InteractButtonCheck = true;
		if( m_pCurrentInteractable->IsA( AC_InteractableTree::StaticClass() )
			|| m_pCurrentInteractable->IsA( AC_SnowInteractable::StaticClass() ) )
		{

			if( m_pCurrentInteractable->IsA( AC_InteractableTree::StaticClass() ) )
			{
				m_CurrentState = EPlayerStates::ES_CLIMBING_UP;
			}
			else
			{
				m_CurrentState = EPlayerStates::ES_SNOW;
			}
		}
	}
}

void AC_Player::Moving( float DeltaTime )
{
	bool GotInput = GetInput( DeltaTime );

	if( !m_pAnimInstance->IsPlaying( m_pRunAnimation ) || !m_pAnimInstance->IsPlaying() )
	{
		m_pAnimInstance->ChangeAnimation( m_pRunAnimation );
	}

	if( m_pCurrentInteractable != nullptr && m_DistanceToInteractable < 1000 && IsPlayerInteracting )
	{
		m_InteractButtonCheck = true;
		if( m_pCurrentInteractable->IsA( AC_InteractableTree::StaticClass() )
			|| m_pCurrentInteractable->IsA( AC_SnowInteractable::StaticClass() ) )
		{

			if( m_pCurrentInteractable->IsA( AC_InteractableTree::StaticClass() ) )
			{
				m_CurrentState = EPlayerStates::ES_CLIMBING_UP;

			}
			else
			{
				m_CurrentState = EPlayerStates::ES_SNOW;
			}
		}
	}

	if( !GotInput )
	{
		m_CurrentState = EPlayerStates::ES_IDLE;
	}

	if( m_IsJumping )
	{
		m_CurrentState = EPlayerStates::ES_JUMP;
	}
}

void AC_Player::Dead( float DeltaTime )
{
	if( !m_pAnimInstance->IsPlaying( m_pDieAnimation ) )
	{
		m_pAnimInstance->ChangeAnimation( m_pDieAnimation, false );
	}

	if( !m_pAnimInstance->IsPlaying() )
	{
		m_Dead = true;
	}
}

void AC_Player::Howling( float DeltaTime )
{
	if( m_pHowlingAnimation )
	{
		if( !m_pAnimInstance->IsPlaying( m_pHowlingAnimation ) )
		{
			m_pAnimInstance->ChangeAnimation( m_pHowlingAnimation, false );
			m_MoonstoneCharges = 0;
			m_pMoonstoneUI->ChangeMoonstoneCharges( m_MoonstoneCharges );
			OnHowl();
		}
		else if( !m_pAnimInstance->IsPlaying() )
		{
			if( m_MovementDirection.Length() == 0 )
				m_CurrentState = EPlayerStates::ES_IDLE;
			else
				m_CurrentState = EPlayerStates::ES_MOVING;

			m_pGoalIndicator->ChangeVisibility( true );
		}
	}
}

void AC_Player::CallOnEnemies()
{
	TArray<AActor*> EnemyArray;
	UGameplayStatics::GetAllActorsOfClass( m_pWorld, m_pEnemyClass, EnemyArray );

	FVector PlayerPos = GetActorLocation();

	for( AActor* pEnemy : EnemyArray )
	{
		FVector PlayerToEnemy = pEnemy->GetActorLocation() - PlayerPos;
		double Distance = PlayerToEnemy.Length();

		if( Distance < m_CallOnEnemyRadius )
			Cast<AC_Enemy>( pEnemy )->CallOn( PlayerPos );
	}
}

void AC_Player::Jump( float DeltaTime )
{
	if( !m_pAnimInstance->IsPlaying( m_pJumpAnimation ) )
	{
		m_pAnimInstance->ChangeAnimation( m_pJumpAnimation, false );
		OnJump();
	}
	
	m_pCapsuleCollider->AddRelativeLocation( FVector( 0, 0, m_JumpSpeed ) );
	m_JumpSpeed -= 9.82 * DeltaTime;

	if( !m_IsJumping )
	{
		if (m_MovementDirection.Length() == 0)
			m_CurrentState = EPlayerStates::ES_IDLE;
		else
			m_CurrentState = EPlayerStates::ES_MOVING;
	}
}

void AC_Player::ClimbUp( float DeltaTime )
{
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), FVector( m_pCurrentInteractable->GetActorLocation().X, m_pCurrentInteractable->GetActorLocation().Y, GetActorLocation().Z ) );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, DeltaTime, m_RotationSpeed );
	SetActorRotation( Rotator );
	float angle = FMath::Acos( FVector::DotProduct( GetActorLocation(), m_pCurrentInteractable->GetActorLocation() ) / ( GetActorLocation().Length() * m_pCurrentInteractable->GetActorLocation().Length() ) );

	if( angle < 10 )
	{

		if( !m_pAnimInstance->IsPlaying( m_pClimbingUpAnimation ) )
		{
			m_pAnimInstance->ChangeAnimation( m_pClimbingUpAnimation, false );
			OnClimbUp();
		}

		if( !m_pAnimInstance->IsPlaying() )
		{
			m_CurrentState = EPlayerStates::ES_HIDING;
		}
	}
}

void AC_Player::Hiding( float DeltaTime )
{
	if( !m_Hiding && !m_pAnimInstance->IsPlaying() )
	{
		m_pPlayerMesh->SetVisibility( false );
		m_Hiding = true;
		m_pCurrentInteractable->SetIsPlayerInteracting( true );
		
		if( m_pCurrentInteractable->IsA( AC_SnowInteractable::StaticClass() ) )
		{
			m_pCurrentInteractable->SetIsPlayerBurning( m_OnFire );
		}
	}

	AC_Intractable* tmpInteractable = m_pCurrentInteractable;

	if( m_pCurrentInteractable->IsA( AC_InteractableTree::StaticClass() ) && !m_OnFire )
	{
		m_OnFire = m_pCurrentInteractable->ShouldPlayerBurn();
	}

	bool GotInput = GetInput( DeltaTime );

	if( !IsPlayerInteracting ) {
		if( tmpInteractable->IsA( AC_InteractableTree::StaticClass() ) )
		{
			m_CurrentState = EPlayerStates::ES_CLIMBING_DOWN;
		}
		else
		{
			m_pPlayerMesh->SetVisibility( true );
			m_pAnimInstance->ChangeAnimation( m_pUnHideAnimation, false );
			m_Hiding = false;

		}
	}
}

void AC_Player::ClimbDown( float DeltaTime )
{
	if( !m_pAnimInstance->IsPlaying( m_pClimbingDownAnimation ) )
	{
		m_pAnimInstance->ChangeAnimation( m_pClimbingDownAnimation, false );
		OnClimbDown();
		m_Hiding = false;
		m_pPlayerMesh->SetVisibility( true );
	}

	if( !m_pAnimInstance->IsPlaying() )
	{
		m_InteractButtonCheck = false;
		if( m_MovementDirection.Length() == 0 )
			m_CurrentState = EPlayerStates::ES_IDLE;
		else
			m_CurrentState = EPlayerStates::ES_MOVING;
	}
}

void AC_Player::InteractSnow( float DeltaTime )
{
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_pCurrentInteractable->GetActorLocation() );
	Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, DeltaTime, m_RotationSpeed );
	SetActorRotation( Rotator );
	float angle = FMath::Acos( FVector::DotProduct( GetActorLocation(), m_pCurrentInteractable->GetActorLocation() ) / ( GetActorLocation().Length() * m_pCurrentInteractable->GetActorLocation().Length() ) );

	if( angle < 10 )
	{
		if( !m_pAnimInstance->IsPlaying( m_pHideAnimation ) )
		{
			m_pAnimInstance->ChangeAnimation( m_pHideAnimation, false );
			OnSnowDiveIn();
		}

		m_pCurrentInteractable->SetIsPlayerInteracting( true );
		m_pCurrentInteractable->SetIsPlayerBurning( m_OnFire );

		if( m_OnFire )
		{
			m_OnFire = m_pCurrentInteractable->ShouldPlayerBurn();
		}


		if( !m_pAnimInstance->IsPlaying() )
		{
			m_Hiding = true;
			m_pPlayerMesh->SetVisibility( false );
			m_CurrentState = EPlayerStates::ES_UNHIDE;
		}
	}
}

void AC_Player::UnHide( float DeltaTime )
{
	bool GotInput = GetInput( DeltaTime );

	if( !m_Hiding && !m_pAnimInstance->IsPlaying() )
	{
		m_interactingSnow = false;
		m_InteractButtonCheck = false;
		if( m_MovementDirection.Length() == 0 )
			m_CurrentState = EPlayerStates::ES_IDLE;
		else
			m_CurrentState = EPlayerStates::ES_MOVING;

	}
	else if( !IsPlayerInteracting && !m_pAnimInstance->IsPlaying() )
	{
		m_pPlayerMesh->SetVisibility( true );
		m_pAnimInstance->ChangeAnimation( m_pUnHideAnimation, false );
		OnSnowDiveOut();
		m_Hiding = false;
	}
}

bool AC_Player::GetInput( float DeltaTime )
{
	bool GotInput = false;

	if( m_MovementDirection.Length() != 0 )
	{
		if( m_CurrentState == EPlayerStates::ES_IDLE )
			m_CurrentState = EPlayerStates::ES_MOVING;

		m_pCamera->SetClicked( false );

		GotInput = true;
	}

	TArray<AActor*> Results;
	m_pInteractionCollider->GetOverlappingActors( Results );

	if( Results.Num() != 0 )
	{
		bool Interactable = false;
		for( AActor* actor : Results )
		{

			if( actor->IsA( AC_Intractable::StaticClass() ) )
			{
				m_pCurrentInteractable = Cast<AC_Intractable>( actor );
				Interactable = true;
			}
		}

		if( Interactable == false )
			m_pCurrentInteractable = nullptr;
	}

	if( m_pCurrentInteractable != nullptr )
	{
		m_DistanceToInteractable = ( m_pCurrentInteractable->GetActorLocation() - GetActorLocation() ).Length();
	}

	return GotInput;
}

void AC_Player::Trapped( float DeltaTime )
{
	if( m_pTrappedAnimation )
	{
		if( !m_pAnimInstance->IsPlaying( m_pTrappedAnimation ) || !m_pAnimInstance->IsPlaying() )
		{
			m_pAnimInstance->ChangeAnimation( m_pTrappedAnimation, false );
			CallOnEnemies();
		}
	}
}

void AC_Player::OnFire( float DeltaTime )
{
	if( m_pFireEffect && m_FireEffectComp == nullptr )
	{
		m_FireEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached( m_pFireEffect, m_pPlayerMesh,
																		 NAME_None, FVector( 0 ), FRotator( 0 ), EAttachLocation::KeepRelativeOffset, true );

		m_FireEffectComp->SetFloatParameter( "SpawnRate", m_FireIntensity );

		m_FireEffectComp->Activate();
	}

	m_FireTimer += DeltaTime;
	m_FireDamageTimer += DeltaTime;
	m_FireSoundTimer += DeltaTime;

	if( m_FireSoundTimer > m_FireSoundDuration )
	{
		WhileOnFire();
		m_FireSoundTimer = 0;
	}

	if( m_FireTimer > m_TimeUntilFireIsOut )
	{
		m_FireTimer = 0;
		m_FireDamageTimer = 0;
		m_OnFire = false;
		return;
	}

	if( m_FireDamageTimer > m_TimeBetweenFireDamage )
	{
		TakeDamage();
		m_FireDamageTimer = 0;
	}

}

void AC_Player::AddMoonstoneCharge()
{
	if( m_MoonstoneCharges < m_MaxMoonstoneCharges )
	{
		m_MoonstoneCharges++;
	}

	OnPickupMoonStone();
	m_pMoonstoneUI->ChangeMoonstoneCharges( m_MoonstoneCharges );
}

FVector AC_Player::GetUnoccupiedPos()
{

	FVector TmpVec = FVector( 0, 0, 0 );
	bool FoundOne = false;

	for( int i = 0; i < m_pEnemyPositions.Num(); i++ )
	{
		if( m_pEnemyPositions[i].m_Occupied == false && !FoundOne )
		{
			m_pEnemyPositions[i].m_Occupied = true;
			TmpVec = m_pEnemyPositions[i].m_Pos;
			FoundOne = true;
		}
	}

	return TmpVec;
}

void AC_Player::LetGoOfPos( FVector pos )
{
	for( int i = 0; i < m_pEnemyPositions.Num(); i++ )
	{
		if( m_pEnemyPositions[i].m_Pos == pos )
		{
			m_pEnemyPositions[i].m_Occupied = false;
		}
	}
}

void AC_Player::UntrapPlayer()
{
	if( m_MovementDirection.Length() == 0 )
		m_CurrentState = EPlayerStates::ES_IDLE;
	else
		m_CurrentState = EPlayerStates::ES_MOVING;
}

void AC_Player::TakeDamage()
{
	m_CurrentHealth--;

	OnTakenDamage();

	if( m_CurrentHealth <= 0 )
	{
		m_CurrentState = EPlayerStates::ES_DEAD;
	}

	m_pMoonstoneUI->SetDamageIndicator( m_CurrentHealth );
}

void AC_Player::RestoreHealth()
{
	if( m_CurrentHealth < m_MaxHealth )
	{
		m_CurrentHealth++;
	}

	m_pMoonstoneUI->SetDamageIndicator( m_CurrentHealth );
}

void AC_Player::MoveRightLeft( float value )
{
	m_MovementDirection.Y = value;
}

void AC_Player::MoveForwardBackwards( float value )
{
	m_MovementDirection.X = value;
}

// Called every frame
void AC_Player::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	StateMachine( DeltaTime );

	FVector ActorLocation = GetActorLocation();
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor( this );
	float Offset = m_pCapsuleCollider->GetScaledCapsuleRadius();
	m_pWorld->LineTraceSingleByChannel( HitResult, ActorLocation + FVector( 0, 0, 50 ), ActorLocation - FVector( 0, 0, 10000 ), ECC_Visibility, Params );
	if( HitResult.GetActor() && HitResult.GetActor()->ActorHasTag( "Walkable" ) )
	{
		if( !m_IsJumping || ( HitResult.Distance < 200 && m_JumpSpeed < 0 ) )
		{
			float Height = m_pCapsuleCollider->GetScaledCapsuleHalfHeight() + 40;
			FVector Location = FMath::VInterpTo( ActorLocation, HitResult.ImpactPoint + FVector( 0, 0, Height ), DeltaTime, 10 );
			Location.X = ActorLocation.X;
			Location.Y = ActorLocation.Y;
			SetActorLocation( Location );
			m_IsJumping = false;
		}
	}
	else if( HitResult.GetActor() )
	{
		AddActorWorldOffset( GetActorForwardVector() * 1 );
	}

	if( m_OnFire )
		OnFire( DeltaTime );

	if( m_pFireEffect && m_FireEffectComp )
	{
		if( !m_OnFire && m_FireEffectComp->IsActive() )
		{
			m_FireEffectComp->Deactivate();
			m_FireEffectComp = nullptr;
			m_OnFire = false;
		}
	}

	if( m_CurrentHealth <= 0 )
	{
		m_CurrentState = EPlayerStates::ES_DEAD;
	}

	FVector Forward;
	FVector Right;
	FVector MovementDirection( 0 );
	if( !m_IsJumping )
	{
		MovementDirection = m_MovementDirection;
		m_tmpMovementDirection = m_MovementDirection;
		Forward = m_pCamera->GetActorForwardVector();
		Right = m_pCamera->GetActorRightVector();
	}
	else
	{
		if( m_tmpMovementDirection.Length() > 0 )
			MovementDirection.X = 1;
		Forward = GetActorForwardVector();
		Right = GetActorForwardVector();
	}
	if( m_CurrentState == EPlayerStates::ES_MOVING || m_CurrentState == EPlayerStates::ES_JUMP )
	{
		if( MovementDirection.Length() != 0 )
		{
			Forward.Z = 0;
			Forward.Normalize();
			m_RotationDirection = MovementDirection;
			MovementDirection.Normalize();

			FVector NewLocation( 0 );
			NewLocation += Forward * MovementDirection.X;
			NewLocation += Right * MovementDirection.Y;
			NewLocation *= m_MovementSpeed * DeltaTime;
			NewLocation += GetActorLocation();
			SetActorLocation( NewLocation, true );

			m_LookAtPosition = GetActorLocation() + ( m_pCamera->GetActorRightVector() * m_RotationDirection.Y ) + ( Forward * m_RotationDirection.X );

			if( m_CurrentState != EPlayerStates::ES_JUMP )
			{
				m_FootstepTimer += DeltaTime;
				if( m_FootstepTimer > m_TimeBetweenFootsteps )
				{
					m_FootstepTimer = 0;
					OnFootstep();
				}
			}
		}
		else
		{
			m_FootstepTimer = 0;
		}

		FRotator Rotator = UKismetMathLibrary::FindLookAtRotation( GetActorLocation(), m_LookAtPosition );
		Rotator = FMath::RInterpTo( GetActorRotation(), Rotator, DeltaTime, m_RotationSpeed );

		if( !m_IsJumping )
			SetActorRotation( Rotator );
	}

	if( m_CurrentState != EPlayerStates::ES_IDLE && m_CurrentState != EPlayerStates::ES_MOVING && m_CurrentState != EPlayerStates::ES_JUMP )
	{
		m_IsJumping = false;
	}

	if( m_pGoalIndicator->IsVisible() ) {

		m_HowlTimer += DeltaTime;

		FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation( m_pCamera->GetActorLocation(), m_GoalPos );

		FRotator TmpRotator = LookAtRotator - m_pCamera->GetActorRotation();

		m_pGoalIndicator->SetDirection( TmpRotator.Yaw );

		if( m_HowlTimer >= m_MaxTimeHowl )
		{
			m_HowlTimer = 0;
			m_pGoalIndicator->ChangeVisibility( false );
		}
	}

	m_ChasedSoundTimer += DeltaTime;
	if( m_ChasedSoundTimer > m_ChasedSoundDuration )
	{
		if( m_IsChased )
			WhileChased();
	}

	m_NotChasedSoundTimer += DeltaTime;
	if( m_NotChasedSoundTimer > m_NotChasedSoundDuration )
	{
		if( !m_IsChased )
			WhileNotChased();
	}
	m_IsChased = false;

	FRotator Rotator = GetActorRotation();
	Rotator.Pitch = 0;
	SetActorRotation( Rotator );
}
