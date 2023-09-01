// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <Engine/SkeletalMesh.h>
#include <Engine/World.h>
#include <Engine/EngineTypes.h>
#include <NavigationSystem.h>
#include <game/Player/Camera/C_Camera.h>
#include <Components/CapsuleComponent.h>
#include "AITypes.h"
#include <Components/InputComponent.h>
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <game/Interactable/C_Intractable.h>
#include "game/Interactable/Tree/C_InteractableTree.h"

#include "C_Player.generated.h"

UENUM()
enum class EPlayerStates // class removes intellisense error
{
	ES_IDLE,
	ES_MOVING,
	ES_CLIMBING_UP,
	ES_CLIMBING_DOWN,
	ES_HIDING,
	ES_DEAD,
	ES_TRAPPED,
	ES_HOWLING,
	ES_SNOW,
	ES_UNHIDE,
	ES_JUMP

};

USTRUCT()
struct FEnemyPosition {

	GENERATED_BODY()

		UPROPERTY( VisibleAnywhere, Category = "Enemy Positions" )
		FVector m_Pos;

	UPROPERTY( VisibleAnywhere, Category = "Enemy Positions" )
		bool m_Occupied;
};


class AAIController;
class UFloatingPawnMovement;
class UC_GoalIndicator_Widget;
class UC_MoonstoneUI;
class UNiagaraSystem;
class UNiagaraComponent;
class UC_AnimInstance;

UCLASS()
class GAME_API AC_Player : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AC_Player();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void StateMachine( float DeltaTime );

	UFUNCTION()
		void Idle( float DeltaTime );

	UFUNCTION()
		void Moving( float DeltaTime );

	UFUNCTION()
		void Dead( float DeltaTime );

	UFUNCTION()
		void ClimbUp( float DeltaTime );

	UFUNCTION()
		void Hiding( float DeltaTime );

	UFUNCTION()
		void ClimbDown( float DeltaTime );

	UFUNCTION()
		void InteractSnow( float DeltaTime );

	UFUNCTION()
		void UnHide( float DeltaTime );

	UFUNCTION()
		bool GetInput( float DeltaTime );

	UFUNCTION()
		void Trapped( float DeltaTime );

	UFUNCTION()
		void OnFire( float DeltaTime );

	UFUNCTION()
		void Howling( float DeltaTime );

	UFUNCTION()
		void CallOnEnemies();

	UFUNCTION()
		void Jump( float DeltaTime );

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "Sound Events" )
		void OnTakenDamage();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void WhileOnFire();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnHowl();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnClimbUp();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnClimbDown();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnSnowDiveIn();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnSnowDiveOut();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnPickupMoonStone();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnStuckInBeartrap();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnFootstep();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void WhileChased();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void WhileNotChased();
	UFUNCTION( BlueprintImplementableEvent, Category = "Sound Events" )
		void OnJump();

	UPROPERTY( EditDefaultsOnly )
		USkeletalMeshComponent* m_pPlayerMesh;
	UPROPERTY( EditDefaultsOnly )
		UCapsuleComponent* m_pCapsuleCollider;

	UPROPERTY( EditDefaultsOnly )
		UCapsuleComponent* m_pInteractionCollider;

	UWorld* m_pWorld;
	AC_Camera* m_pCamera;
	AAIController* m_pAIController;
	UFloatingPawnMovement* m_pFloatingPawnMovement;
	UPROPERTY( EditDefaultsOnly, Category = "Movement" )
		float m_MovementSpeed;
	AC_Intractable* m_pCurrentInteractable;

	UPROPERTY( EditAnywhere, Category = "Movement" )
		float m_RotationSpeed;
	float m_DistanceToInteractable;

	FVector m_RotationDirection;
	FVector m_LookAtPosition;
	FVector m_MovementDirection;
	UPROPERTY( EditAnywhere, Category = "Camera Position" )
		FVector m_CameraDistance;

	UPROPERTY( EditAnywhere, Category = "Fire" )
		bool m_OnFire;

	float m_FireTimer;
	float m_FireDamageTimer;

	float m_FireSoundTimer;
	UPROPERTY( EditAnyWhere, BlueprintReadWrite, Category = "Fire Duration" )
		float m_FireSoundDuration;

	UPROPERTY( EditAnyWhere, Category = "Footstep Time" )
		float m_TimeBetweenFootsteps;
	float m_FootstepTimer;

	UPROPERTY( EditAnyWhere, Category = "Chased Sound Duration" )
		float m_ChasedSoundDuration;
	float m_ChasedSoundTimer;

	UPROPERTY( EditAnyWhere, Category = "Not Chased Sound Duration" )
		float m_NotChasedSoundDuration;
	float m_NotChasedSoundTimer;

	UPROPERTY( EditAnywhere, Category = "Fire" )
		float m_TimeBetweenFireDamage;

	UPROPERTY( EditAnywhere, Category = "Fire" )
		float m_TimeUntilFireIsOut;

	UPROPERTY( EditAnywhere, Category = "Fire" )
		UNiagaraSystem* m_pFireEffect;

	UNiagaraComponent* m_FireEffectComp;

	UPROPERTY( EditAnywhere, Category = "Fire" )
		float m_FireIntensity;

	UPROPERTY( EditAnywhere, Category = "Climb" )
		float m_ClimbHeight;

	UPROPERTY( EditDefaultsOnly )
		EPlayerStates m_CurrentState;

	UPROPERTY( EditAnywhere, Category = "Health" )
		int m_MaxHealth;

	UPROPERTY( EditAnywhere, Category = "Health" )
		int m_CurrentHealth;

	UPROPERTY( EditAnywhere, Category = "Moonstone" )
		int m_MoonstoneCharges;

	UPROPERTY( EditAnywhere, Category = "Moonstone" )
		int m_MaxMoonstoneCharges;

	UPROPERTY( EditAnywhere, Category = "Moonstone" )
		float m_HowlTimer;

	UPROPERTY( EditAnywhere, Category = "Moonstone" )
		float m_MaxTimeHowl;

	bool m_Hiding;
	bool m_Dead;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pClimbingUpAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pClimbingDownAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pIdleAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pRunAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pDieAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pTrappedAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pHowlingAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pHideAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pUnHideAnimation;

	UPROPERTY( EditDefaultsOnly, Category = "Animations" )
		UAnimationAsset* m_pJumpAnimation;

	bool m_interactingSnow;
	FVector m_groundlevel;

	UPROPERTY( EditAnywhere, Category = "Widgets" )
		TSubclassOf<UC_GoalIndicator_Widget> m_GoalIndicatorClass;

	FVector m_GoalPos;

	UPROPERTY()
		UC_GoalIndicator_Widget* m_pGoalIndicator;

	UPROPERTY( EditAnywhere, Category = "Widgets" )
		TSubclassOf<UC_MoonstoneUI> m_MoonstoneUIClass;

	UPROPERTY()
		UC_MoonstoneUI* m_pMoonstoneUI;

	UPROPERTY( EditAnywhere, Category = "Sound" )
		float m_CallOnEnemyRadius; // Radius for how far howl & stuck in beartrap sound reach
	UClass* m_pEnemyClass;


	UClass* m_pCameraClass;

	bool IsPlayerInteracting;

	bool m_InteractButtonCheck;

	FVector m_PlayerStartPos;

	bool m_OnIce;

	UC_AnimInstance* m_pAnimInstance;

	UPROPERTY( EditAnywhere, Category = "Enemy Positions" )
		int m_AmountOfPos;

	UPROPERTY( VisibleAnywhere, Category = "Enemy Positions" )
		int m_MaxAmountOfPos;

	UPROPERTY( EditAnywhere, Category = "Enemy Positions" )
		float m_InnerRadius;

	UPROPERTY( EditAnywhere, Category = "Enemy Positions" )
		float m_OuterRadius;

	UPROPERTY( VisibleAnywhere, Category = "Enemy Positions" )
		TArray<FEnemyPosition> m_pEnemyPositions;

	float m_JumpSpeed;

	float m_OriginalJumpSpeed;

	bool m_IsJumping;

	FVector m_tmpMovementDirection;

	bool m_IsChased;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	bool IsPlayerHiding() { return m_Hiding; }
	bool IsDead() { return m_Dead; }

	void TrapPlayer() { m_CurrentState = EPlayerStates::ES_TRAPPED; OnStuckInBeartrap(); }
	void UntrapPlayer();

	void SetOnFire() { m_OnFire = true; }
	AC_InteractableTree* GetUsedInteractableTree() { return Cast<AC_InteractableTree>( m_pCurrentInteractable ); }
	
	void TakeDamage();

	void RestoreHealth();

	UFUNCTION( BlueprintCallable )
		void MoveRightLeft( float value );

	UFUNCTION( BlueprintCallable )
		void MoveForwardBackwards( float value );

	void AddMoonstoneCharge();

	int GetMoonstoneCharges() { return m_MoonstoneCharges; }
	int GetMaxMoonstoneCharges() { return m_MaxMoonstoneCharges; }

	void SetPlayerInteracting( bool _IsPlayerInteracting ) { IsPlayerInteracting = _IsPlayerInteracting; }
	void SetGoalPos( FVector newVec ) { m_GoalPos = newVec; }

	USkeletalMeshComponent* GetPlayerMesh() { return m_pPlayerMesh; }

	void StartHowl() { m_CurrentState = EPlayerStates::ES_HOWLING; }

	FVector GetUnoccupiedPos();

	void LetGoOfPos( FVector pos );

	AC_Camera* GetCamera() { return m_pCamera; }

	void SetIsJumping( bool isJumping ) { m_IsJumping = isJumping; }

	void ResetJumpSpeed() { m_JumpSpeed = m_OriginalJumpSpeed; }

	bool IsPlayerInAir() { return m_IsJumping; }

	bool GetInteractButtonCheck() { return m_InteractButtonCheck; }

	EPlayerStates GetCurrentState() { return m_CurrentState; }
	
	void SetIsChased() { m_IsChased = true; }
};
