// Fill out your copyright notice in the Description page of Project Settings.


#include "GretaAiController.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"

AGretaAiController::AGretaAiController(FObjectInitializer const& ObjectIntializer)
{
	HearingRange = 800.0f;
	HeardSound = false;
	SetupPerception();
}

void AGretaAiController::ChangeHearingRange(float NewValue)
{
	HearingRange = NewValue;
	HearingConfig->HearingRange = HearingRange;
	GetPerceptionComponent()->RequestStimuliListenerUpdate();
};

void AGretaAiController::OnUpdated(const TArray<AActor*>& UpdatedActors)
{
	HeardSound = true;
}

void AGretaAiController::SetupPerception()
{
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->HearingRange = HearingRange;

	GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AGretaAiController::OnUpdated);
	GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	
}
