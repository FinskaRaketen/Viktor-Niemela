// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include <game/Interactable/C_Intractable.h>
#include <Components/BoxComponent.h>
#include "C_Camera.generated.h"

class AC_Player;

class UCameraComponent;

UCLASS()
class GAME_API AC_Camera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AC_Camera();

	void ClickedOnScreen();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY( EditDefaultsOnly )
		UCameraComponent* m_pCamera;

	AC_Player* m_pPlayer;

	float m_HitresultDistance;
	FVector2D m_Mousepos;
	UWorld* m_pWorld;
	FVector3d m_MovePosition;

	UPROPERTY( EditAnywhere, Category = "MouseScroll" )
		float m_MaxCameraDistance;
	UPROPERTY( EditAnywhere, Category = "MouseScroll" )
		float m_MinCameraDistance;
	float m_ScrollValue;
	float m_ScrollSpeed;

	bool m_Clicked;
	AC_Intractable* m_pCurrentInteractable;

	TArray<UObject> m_CollisionObject;
	TArray<FHitResult> m_MovePositionCollisionHits;

	void CheckCollision( FVector retPoint, FVector TargetPos, float Deltatime );

	/**
* Changes how close the collisionBox for transpharency is to the player
* @param The Higher the value, the further away the collisionBox is to Player.
*/


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Camera Occlusion|Materials" )
		UMaterialInterface* FadeMaterial;





	UPROPERTY( EditAnywhere )
		float m_CameraDistance;

	bool m_once;


	FVector2D m_CameraInput;

	FVector m_OffsetFromPlayer;

	UPROPERTY( EditAnywhere, Category = "Camera Rotation" )
		float m_MinCameraClamp;

	UPROPERTY( EditAnywhere, Category = "Camera Rotation" )
		float m_MaxCameraClamp;

	FVector PlayerLocation;

	bool m_IsPaused;

	

private:
	UClass* m_pPlayerClass;

public:

	FVector3d GetNewPos() { return m_MovePosition; }
	bool GetClicked() { return m_Clicked; }
	void SetClicked( bool Clicked ) { m_Clicked = Clicked; }

	AC_Intractable* GetCurrentInteractable() { return m_pCurrentInteractable; }

	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent( class UInputComponent* PlayerInputComponent ) override;

	void HorizontalMovement( float value );

	void VerticalMovement( float value );

	void Anykey( FKey key );

	void Howl();

	void CameraX( float value );

	void CameraY( float value );

	void IsInteracting();

	void IsNotInteracting();

	void PauseGame();

	void SetIsPaused( bool paused ) { m_IsPaused = paused; }

	bool IsPaused() { return m_IsPaused; }

	void PlayerJump();
};

