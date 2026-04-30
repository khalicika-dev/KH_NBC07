// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AMyPlayerController::AMyPlayerController()
{
	//Load InputActions
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Asset = TEXT("/Game/Input/IMC_Flight.IMC_Flight");
	if (IMC_Asset.Succeeded())
		InputMappingContext = IMC_Asset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Move_Asset = TEXT("/Game/Input/IA_Move.IA_Move");
	if (IA_Move_Asset.Succeeded())
		MoveAction = IA_Move_Asset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Look_Asset = TEXT("/Game/Input/IA_Look.IA_Look");
	if (IA_Look_Asset.Succeeded())
		LookAction = IA_Look_Asset.Object;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
