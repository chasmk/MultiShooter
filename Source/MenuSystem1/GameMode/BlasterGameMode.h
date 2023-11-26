// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern MENUSYSTEM1_API const FName Cooldown; //自己添加的MatchState  Match duration has been reached. Display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class MENUSYSTEM1_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	//下面存放的是time的初始值，可以在蓝图里更新
	UPROPERTY(EditDefaultsOnly, Category="Game Time")
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Game Time")
	float MatchTime = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Game Time")
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	/**
	 * @brief 每次MatchState更改后都会调用
	 */
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
