// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MenuSystem1/PlayerController/BlasterPlayerController.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MENUSYSTEM1_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;
	void Heal(float HealAmount, float HealingTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
	
protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/**
	 * Heal buff
	 */

	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	/**
	 * Speed buff
	*/

	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	float SpeedBuffTime;
	

	/**
	 * @brief 由于OnSphereOverlap只会在server上调用，所以只会更改server上的速度
	 * @param BaseSpeed 
	 * @param CrouchSpeed 
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/**
	 * Jump buff
	 */

	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

	/**
	 * Buff Bar
	 */
	void HandleBuffHUD(float BuffTime);//在HUD上显示buff的剩余时间条
	void BuffRampUp(float DeltaTime);
	bool isUseBuff = false;
	float BuffBarRate = 0.f;
	float CurBarAmount = 0.f;
	class ABlasterPlayerController* BlasterPlayerController;
	
public:
};
