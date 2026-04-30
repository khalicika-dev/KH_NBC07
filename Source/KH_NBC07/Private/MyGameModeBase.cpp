// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "Flight.h"
#include "MyPlayerController.h"

AMyGameModeBase::AMyGameModeBase()
{
	DefaultPawnClass = AFlight::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}
