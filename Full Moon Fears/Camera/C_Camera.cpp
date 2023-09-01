// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Camera.h"
#include "game/Player/C_Player.h"
#include <Camera/CameraComponent.h>
#include <Components/SphereComponent.h>
#include <Kismet/GameplayStatics.h>
#include <UObject/ConstructorHelpers.h>

// Sets default values
AC_Camera::AC_Camera()
	: m_Mousepos( 0.0f, 0.0f )
	, m_HitresultDistance( 100000.0f )
	, m_pCamera( nullptr )
	, m_MovePosition( 0.0, 0.0, 0.0 )
	, m_pWorld( nullptr )
	, m_ScrollValue( 0.0f )
	, m_MaxCameraDistance( 100.0f )
	, m_MinCameraDistance( -100.0f )
	, m_ScrollSpeed( 4.0f )
	, m_Clicked( false )
	, m_pPlayer( nullptr )
	, m_pCurrentInteractable( nullptr )
	, m_once( false )
	, m_CameraDistance( 2.0f )
	, m_pPlayerClass( nullptr )
	, m_MaxCameraClamp( 15.0f )
	, m_MinCameraClamp( -75.0f )
	, m_IsPaused( false )
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pCamera = CreateDefaultSubobject<UCameraComponent>( TEXT( "Camera" ) );
	m_CameraInput = FVector2D( 0, 0 );

	// TEXT: Can be found by right-clicking blueprint and copy reference;
	static ConstructorHelpers::FObjectFinder<UClass> PlayerClass( TEXT( "Class'/Script/game.C_Player'" ) );
	if( PlayerClass.Object )
		m_pPlayerClass = PlayerClass.Object;
}

void AC_Camera::ClickedOnScreen()
{
	ULocalPlayer* localplayer = GetWorld()->GetGameInstance()->GetLocalPlayerByIndex( 0 );

	FRotator Rotator;
	FVector Location;
	FRenderTarget* rendertarget = m_pWorld->GetGameViewport()->Viewport;
	FEngineShowFlags& Flags = m_pWorld->GetGameViewport()->EngineShowFlags;
	FSceneViewFamily FSceneViewFamily( FSceneViewFamily::ConstructionValues( rendertarget, m_pWorld->Scene, Flags ) );
	if( localplayer != NULL )
	{
		FVector MouseInWorldPosition;
		FVector WorldDirection;
		FCollisionQueryParams Collisionquery;

		FSceneView* SceneView = localplayer->CalcSceneView( &FSceneViewFamily, Location, Rotator, m_pWorld->GetGameViewport()->Viewport );

		if( m_pWorld->GetGameViewport()->GetMousePosition( m_Mousepos ) )
		{
			SceneView->DeprojectFVector2D( m_Mousepos, MouseInWorldPosition, WorldDirection );
			m_pWorld->LineTraceMultiByChannel( m_MovePositionCollisionHits, MouseInWorldPosition, MouseInWorldPosition + WorldDirection * m_HitresultDistance, ECC_WorldDynamic, Collisionquery );
			if( m_MovePositionCollisionHits.Num() <= 0 )
				return;

			int Index = 0;

			for( FHitResult& rHitResult : m_MovePositionCollisionHits )
			{
				if( Cast<USphereComponent>( rHitResult.GetComponent() ) )
					continue;

				AC_Intractable* tmpInteractable = Cast<AC_Intractable>( m_MovePositionCollisionHits[Index].GetActor() );
				if( tmpInteractable == nullptr && m_pCurrentInteractable != nullptr )
				{
					m_pCurrentInteractable->SetIsPlayerInteracting( false );
				}

				m_pCurrentInteractable = tmpInteractable;

				m_Clicked = true;
			}
		}
	}
}

void AC_Camera::CheckCollision( FVector retPoint, FVector TargetPos, float Deltatime )
{
	SetActorLocation( retPoint );
}


// Called when the game starts or when spawned
void AC_Camera::BeginPlay()
{
	Super::BeginPlay();

	m_pWorld = GetWorld();

	m_pPlayer = Cast<AC_Player>( UGameplayStatics::GetActorOfClass( m_pWorld, m_pPlayerClass ) );
	PlayerLocation = m_pPlayer->GetPlayerMesh()->GetComponentLocation();
	m_OffsetFromPlayer = GetActorLocation() + GetActorForwardVector() * ( PlayerLocation.X - GetActorLocation().X );
	m_OffsetFromPlayer -= PlayerLocation;
	FVector LookAtPosition( PlayerLocation );
	LookAtPosition += GetActorRightVector() * m_OffsetFromPlayer.Y;
	LookAtPosition += GetActorUpVector() * m_OffsetFromPlayer.Z;
	m_CameraDistance = ( LookAtPosition - GetActorLocation() ).Length();

}

// Called every frame
void AC_Camera::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	DrawDebugSphere( m_pWorld, m_MovePosition, 5.0f, 15.0f, FColor::Green );
	DrawDebugLine( GetWorld(), GetActorLocation(), PlayerLocation, FColor::Blue );

	PlayerLocation = m_pPlayer->GetPlayerMesh()->GetComponentLocation();
	m_once = true;
	FVector LookAtPosition( PlayerLocation );
	LookAtPosition += GetActorRightVector() * m_OffsetFromPlayer.Y;
	LookAtPosition.Z += m_OffsetFromPlayer.Z;

	CheckCollision( LookAtPosition - GetActorForwardVector() * m_CameraDistance, LookAtPosition, DeltaTime );
}

// Called to bind functionality to input
void AC_Camera::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	PlayerInputComponent->BindAction( "MouseLeftClick", IE_Pressed, this, &AC_Camera::ClickedOnScreen );
	PlayerInputComponent->BindAction( "SpaceBar", IE_Pressed, this, &AC_Camera::Howl );
	PlayerInputComponent->BindAction( "Interact", IE_Pressed, this, &AC_Camera::IsInteracting );
	PlayerInputComponent->BindAction( "Interact", IE_Released, this, &AC_Camera::IsNotInteracting );
	PlayerInputComponent->BindAction( "Jump", IE_Pressed, this, &AC_Camera::PlayerJump );
	PlayerInputComponent->BindAction( "Pause", IE_Pressed, this, &AC_Camera::PauseGame );

	PlayerInputComponent->BindAxis( "Horizontal", this, &AC_Camera::HorizontalMovement );
	PlayerInputComponent->BindAxis( "Vertical", this, &AC_Camera::VerticalMovement );
	PlayerInputComponent->BindAxis( "CameraX", this, &AC_Camera::CameraX );
	PlayerInputComponent->BindAxis( "CameraY", this, &AC_Camera::CameraY );
}

void AC_Camera::HorizontalMovement( float value )
{
	
	m_pPlayer->MoveForwardBackwards( value );
}

void AC_Camera::VerticalMovement( float value )
{
	m_pPlayer->MoveRightLeft( value );
}

void AC_Camera::Anykey( FKey key )
{
	if( key == EKeys::A )
	{
		m_pPlayer->MoveForwardBackwards( 1 );
	}
}

void AC_Camera::Howl()
{
	if( m_pPlayer->GetMoonstoneCharges() == m_pPlayer->GetMaxMoonstoneCharges() )
	{
		m_pPlayer->StartHowl();
	}
}

void AC_Camera::CameraX( float value )
{
	m_CameraInput.X = value;

	FRotator NewCamrotation = GetActorRotation();

	NewCamrotation.Yaw += m_CameraInput.X;

	SetActorRotation( NewCamrotation );
}

void AC_Camera::CameraY( float value )
{
	m_CameraInput.Y = value;

	FRotator NewCamrotation = GetActorRotation();
	NewCamrotation.Pitch = FMath::Clamp( NewCamrotation.Pitch += m_CameraInput.Y, m_MinCameraClamp, m_MaxCameraClamp );

	SetActorRotation( NewCamrotation );
}
void AC_Camera::IsInteracting()
{
	bool IsPlayerVisible = m_pPlayer->GetPlayerMesh()->IsVisible();
	if (!m_pPlayer->GetInteractButtonCheck())
		m_pPlayer->SetPlayerInteracting(true);

	else if ((m_pPlayer->GetCurrentState() == EPlayerStates::ES_HIDING && !IsPlayerVisible) || (m_pPlayer->GetCurrentState() == EPlayerStates::ES_UNHIDE && !IsPlayerVisible))
		m_pPlayer->SetPlayerInteracting(false);
}

void AC_Camera::IsNotInteracting()
{
	if(!m_pPlayer->GetInteractButtonCheck())
	m_pPlayer->SetPlayerInteracting( false );
}

void AC_Camera::PauseGame()
{
	m_IsPaused = true;
}

void AC_Camera::PlayerJump()
{
	if( !m_pPlayer->IsPlayerInAir() )
	{
		m_pPlayer->ResetJumpSpeed();
		m_pPlayer->SetIsJumping( true );
	}
}


