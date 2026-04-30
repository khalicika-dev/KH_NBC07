// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Flight.generated.h"

USTRUCT()
struct FMovingStatus
{
	GENERATED_BODY()
public:
	bool X;	//좌우
	bool Y;	//전후
	bool Z;	//상하
};

UCLASS(Blueprintable)
class KH_NBC07_API AFlight : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFlight();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetSpeed() { return Speed; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetAccel() { return Accel; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetMaxSpeed() { return MaxSpeed; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetLand() { return bLand; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetLandDistance() { return LandDistance; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetMaxLandDistance() { return MaxLandDistance; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetBackSpeedMul() { return BackSpeedMul; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetInputValue() { return InputValue; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);

	UFUNCTION()
	void OnLand(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UCameraComponent> Camera;

	FMovingStatus bMoving;
	FVector Speed;
	const FVector Accel = {600.0f, 800.0f, 800.0f};
	const FVector InertiaDecel = { 300.0f, 100.0f, 150.0f };
	const FVector MaxSpeed = {600.0f, 1600.0f, 400.0f};
	const float BackSpeedMul = 0.5f;
	float LandDistance;
	const float MaxLandDistance = 800.0f;
	bool bLand;
	bool bHasLand;	// 이 프레임에서 착지가 있었는가?


private:
	const float Gravity = -980.0f;
	float Emissive;
	const float AccelEmissive = 30.0f;
	const float MaxEmissive = 30.0f;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> InputContext;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> InputMove;

	UPROPERTY(VisibleAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> InputLook;
	
	float GetLandMul(bool mayLand) { return mayLand ? 0.5f : 1.0f; }
	float GetLandMul() { return bLand ? 0.5f : 1.0f; }
	float GetDistanceToLand();
	
	FVector InputValue;
};
