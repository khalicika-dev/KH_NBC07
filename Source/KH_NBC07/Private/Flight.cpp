// Fill out your copyright notice in the Description page of Project Settings.


#include "Flight.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"

// Sets default values
AFlight::AFlight()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<UBoxComponent>("Root");
	SetRootComponent(Root);
	Root->SetBoxExtent({ 35.0f,27.0f,15.0f });
	Root->SetSimulatePhysics(false);
	Root->SetCollisionProfileName("Pawn");

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(Root);
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SKM_Asset = TEXT("/Game/Flight/Mesh/RGray1.RGray1");
	static ConstructorHelpers::FClassFinder<UAnimInstance> ABP_Asset = TEXT("/Game/Flight/ABP_Flight.ABP_Flight_C");
	if (SKM_Asset.Succeeded())
	{
		Mesh->SetSkeletalMesh(SKM_Asset.Object);
		if (ABP_Asset.Succeeded())
			Mesh->SetAnimInstanceClass(ABP_Asset.Class);
	}
	Mesh->SetRelativeLocation({ 20.0f,0.0f,-15.0f });
	Mesh->SetSimulatePhysics(false);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring");
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 300.0f;
	SpringArm->SocketOffset = { 0,0,10.0f };

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void AFlight::BeginPlay()
{
	Super::BeginPlay();
	Root->OnComponentHit.AddDynamic(this, &AFlight::OnLand);

	Speed = { 0.0f, MaxSpeed.Y / 2.0f,0.0f };
	bMoving = { false,false,false };
	Emissive = MaxEmissive;
	bLand = false;
	bHasLand = false;
	LandDistance = FLT_MAX;
	InputValue = FVector::Zero();
}

void AFlight::Move(const FInputActionValue& value)
{
	if (!Controller) return;
	
	const FVector MoveInput = value.Get<FVector>();
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		InputValue.Y = MoveInput.Y;

		Speed.Y += (MoveInput.Y > 0 ? Accel.Y * MoveInput.Y * GetLandMul() : Accel.Y * MoveInput.Y * BackSpeedMul * GetLandMul()) * GetWorld()->GetDeltaSeconds();
		Speed.Y = FMath::Clamp(Speed.Y, -MaxSpeed.Y * BackSpeedMul * GetLandMul(), MaxSpeed.Y * GetLandMul());
		bMoving.Y = true;
	}
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		InputValue.X = MoveInput.X;

		Speed.X += Accel.X * MoveInput.X * GetWorld()->GetDeltaSeconds();
		Speed.X = FMath::Clamp(Speed.X, -MaxSpeed.X * GetLandMul(), MaxSpeed.X * GetLandMul());
		bMoving.X = true;
	}
	if (!FMath::IsNearlyZero(MoveInput.Z))
	{
		InputValue.Z = MoveInput.Z;

		bool bOutofRange = Speed.Z < -MaxSpeed.Z;
		Speed.Z += Accel.Z * MoveInput.Z * ((bOutofRange && MoveInput.Z < 0.0f) ? -1.0f : 1.0f) * GetWorld()->GetDeltaSeconds();
		if (!bOutofRange && Speed.Z < -MaxSpeed.Z)
			Speed.Z = -MaxSpeed.Z;
		Speed.Z = FMath::Clamp(Speed.Z, FMath::Min(Gravity, -MaxSpeed.Z), MaxSpeed.Z);
		bMoving.Z = true;
	}

	if (!FMath::IsNearlyZero(MoveInput.Y) || !FMath::IsNearlyZero(MoveInput.Z))
	{
		Emissive += FMath::Abs(FMath::Max(FMath::Abs(MoveInput.Y), FMath::Abs(MoveInput.Z))) * AccelEmissive * GetWorld()->GetDeltaSeconds();
		Emissive = FMath::Min(Emissive, MaxEmissive);
	}
}

void AFlight::Look(const FInputActionValue& value)
{
	if (!Controller) return;
	FVector2D LookInput = value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(LookInput.X))
		AddActorWorldRotation(FRotator(0, 90.0f, 0) * LookInput.X * GetWorld()->GetDeltaSeconds());
	if (!FMath::IsNearlyZero(LookInput.Y))
	{
		FRotator Rot;
		Rot = GetActorRotation();
		Rot.Pitch += 90.0f * LookInput.Y * GetWorld()->GetDeltaSeconds();
		Rot.Pitch = FMath::Clamp(Rot.Pitch, -89.9f, 89.9f);
		SetActorRelativeRotation(Rot);
	}
}

void AFlight::OnLand(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FHitResult TraceHit;
	float height = FMath::Abs(GetActorLocation().Z - Hit.ImpactPoint.Z);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FCollisionResponseParams ResponseParams;

	
	GetWorld()->LineTraceSingleByChannel(
		TraceHit,
		GetActorLocation(),
		GetActorLocation() - FVector(0, 0, height+10.0f),
		ECC_Visibility,
		QueryParams
	);
	if (TraceHit.bBlockingHit)
	{
		Speed.Z = 0.0f;
		bHasLand = true;
	}
}

float AFlight::GetDistanceToLand()
{
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;

	FCollisionResponseParams ResponseParams;


	GetWorld()->LineTraceSingleByChannel(
		TraceHit,
		GetActorLocation(),
		GetActorLocation() - FVector(0, 0, MaxLandDistance),
		ECC_Visibility,
		QueryParams
	);
	if (TraceHit.bBlockingHit)
		return TraceHit.Distance;
	else
		return MaxLandDistance;
}

// Called every frame
void AFlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//불빛
	if (!bMoving.Z && !bMoving.Y)
	{
		Emissive -= AccelEmissive * DeltaTime;
		Emissive = FMath::Max(Emissive, 0.0f);
	}

	//상하이동 및 중력
	AddActorWorldOffset(FVector(0.0f, 0.0f, Speed.Z) * GetWorld()->GetDeltaSeconds(), true);
	if (!bMoving.Z)
	{
		InputValue.Z = 0.0f;
		if (bMoving.Y)
		{
			float decelDirection = (Speed.Z > 0 ? -1.0f : 1.0f);
			Speed.Z += InertiaDecel.Z * decelDirection * DeltaTime;
			if (decelDirection * Speed.Z > 0)
				Speed.Z = 0.0f;
		}
		else if(!bLand)
		{
			Speed.Z += Gravity * DeltaTime;
			if (Speed.Z < Gravity)
				Speed.Z = Gravity;
		}
	}
	else
		bMoving.Z = false;

	// 전후진
	FVector VForward = GetActorForwardVector() * Speed.Y;
	//AddActorWorldOffset(FVector(VForward.X, VForward.Y, 0.001f) * GetWorld()->GetDeltaSeconds(), true);
	//AddActorWorldOffset(FVector(0.0f, 0.0f, VForward.Z) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(VForward.X, 0.0f, 0.0f) + (Speed.Y < 0.0f ? 1.0f : -1.0f) * GetActorForwardVector()) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(0.0f, VForward.Y, 0.0f) + (Speed.Y < 0.0f ? 1.0f : -1.0f) * GetActorForwardVector()) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(0.0f, 0.0f, VForward.Z) + (Speed.Y < 0.0f ? 1.0f : -1.0f) * GetActorForwardVector()) * GetWorld()->GetDeltaSeconds(), true);

	if (!bMoving.Y)
	{
		InputValue.Y = 0.0f;
		float decelDirection = (Speed.Y > 0 ? -1.0f : 1.0f);
		Speed.Y += InertiaDecel.Y * decelDirection * DeltaTime;
		if (decelDirection * Speed.Y > 0)
			Speed.Y = 0.0f;
	}
	else
		bMoving.Y = false;

	// 좌우이동
	FVector VRight = GetActorRightVector() * Speed.X;
	FRotator RRoll = GetActorRotation();
	float TargetRoll = 45.0f * Speed.X / MaxSpeed.X;
	float RRollDir = (RRoll.Roll == TargetRoll) ? 0.0f : ((RRoll.Roll > TargetRoll) ? -1.0f : 1.0f);
	RRoll.Roll += 45.0f * (RRollDir) * DeltaTime;
	if (RRollDir * RRoll.Roll >= TargetRoll)
		RRoll.Roll = TargetRoll;
	//AddActorWorldOffset(FVector(VRight.X, VRight.Y, 0.001f) * GetWorld()->GetDeltaSeconds(), true);
	//AddActorWorldOffset(FVector(0.0f, 0.0f, VRight.Z) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(VRight.X, 0.0f, 0.0f) + (Speed.X < 0.0f ? 1.0f : -1.0f) * GetActorRightVector()) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(0.0f, VRight.Y, 0.0f) + (Speed.X < 0.0f ? 1.0f : -1.0f) * GetActorRightVector()) * GetWorld()->GetDeltaSeconds(), true);
	AddActorWorldOffset((FVector(0.0f, 0.0f, VRight.Z) + (Speed.X < 0.0f ? 1.0f : -1.0f) * GetActorRightVector()) * GetWorld()->GetDeltaSeconds(), true);
	SetActorRotation(RRoll);
	if (!bMoving.X)
	{
		InputValue.X = 0.0f;
		float decelDirection = (Speed.X > 0 ? -1.0f : 1.0f);
		Speed.X += InertiaDecel.X * decelDirection * DeltaTime;
		if (decelDirection * Speed.X > 0)
			Speed.X = 0.0f;
	}
	else
		bMoving.X = false;

	Mesh->SetScalarParameterValueOnMaterials(TEXT("Emissive"), Emissive);

	LandDistance = GetDistanceToLand();

	//----스테이터스 출력----
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta, FString::Printf(TEXT("LandDist: %f"), LandDistance));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Magenta, FString::Printf(TEXT("Land: %s"), bLand ? TEXT("True") : TEXT("False")));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, FString::Printf(TEXT("Pitch = %f, Roll = %f"), GetActorRotation().Pitch, GetActorRotation().Roll));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, FString::Printf(TEXT("Speed = X: %f, Y: %f, Z: %f"), Speed.X, Speed.Y, Speed.Z));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Emerald, TEXT("Move:WASD, Up:Space, Down:LShift, Look:Mouse"));
	if (bHasLand)
	{
		bHasLand = false;
		bLand = true;
	}
	else
	{
		bLand = false;
	}
}

// Called to bind functionality to input
void AFlight::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
				EnhancedInputComponent->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &AFlight::Move);
			if (PlayerController->LookAction)
				EnhancedInputComponent->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &AFlight::Look);
		}
	}
}

