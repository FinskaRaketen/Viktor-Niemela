// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GretaAiController.generated.h"

/**
 * 
 */
UCLASS()
class BURGLAR_API AGretaAiController : public AAIController
{
	GENERATED_BODY()

public:
	AGretaAiController(FObjectInitializer const& ObjectIntializer = FObjectInitializer::Get());

	void ChangeHearingRange(float NewValue);

	bool GetHeardSound() { return HeardSound; }

	void SetHeardSound(bool NewValue) { HeardSound = NewValue; }

private:
	class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
	void OnUpdated(const TArray<AActor*>& UpdatedActors);

	void SetupPerception();

	float HearingRange;

	bool HeardSound;
};
