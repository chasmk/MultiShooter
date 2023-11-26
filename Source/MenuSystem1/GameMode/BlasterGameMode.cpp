// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "MenuSystem1/Character/BlasterCharacter.h"
#include "MenuSystem1/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework//PlayerStart.h"
#include "MenuSystem1/PlayerState/BlasterPlayerState.h"
#include "MenuSystem1/GameState/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;//设置为true后，gamemode会保持在WaitingToStart状态
	UE_LOG(LogTemp, Warning, TEXT("BlasterGameMode initialization done"));
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	ENetMode NetMode1 = GetNetMode();
	UE_LOG(LogTemp, Warning, TEXT("NetMode: %d"), NetMode1);
	
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{//warmup时间结束后，就开始游戏
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{//matchtime时间结束后，就开始游戏
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{//Cooldown时间结束后，就重启游戏
			RestartGame();
		}
	}

}

void ABlasterGameMode::OnMatchStateSet()
{//每次MatchState更新都会调用
	Super::OnMatchStateSet();
	/**
	 * 每次MatchState更新后，就逐个调用PlayerController类的函数去更新HUD
	 */
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{//遍历所有PlayerController
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{//角色类的Receive Damage调用
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && AttackerPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);//攻击者加分
		BlasterGameState->UpdateTopScore(AttackerPlayerState);//更新最高得分者
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);//淘汰者加分
	}


	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();//只会在server上调用，在角色类的recieveDamage里调用
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{//销毁角色
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{//随机选择一个起始点生成角色
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
