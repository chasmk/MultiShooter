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
	bDelayedStart = true;//����Ϊtrue��gamemode�ᱣ����WaitingToStart״̬
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
		{//warmupʱ������󣬾Ϳ�ʼ��Ϸ
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{//matchtimeʱ������󣬾Ϳ�ʼ��Ϸ
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{//Cooldownʱ������󣬾�������Ϸ
			RestartGame();
		}
	}

}

void ABlasterGameMode::OnMatchStateSet()
{//ÿ��MatchState���¶������
	Super::OnMatchStateSet();
	/**
	 * ÿ��MatchState���º󣬾��������PlayerController��ĺ���ȥ����HUD
	 */
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{//��������PlayerController
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{//��ɫ���Receive Damage����
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && AttackerPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);//�����߼ӷ�
		BlasterGameState->UpdateTopScore(AttackerPlayerState);//������ߵ÷���
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);//��̭�߼ӷ�
	}


	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();//ֻ����server�ϵ��ã��ڽ�ɫ���recieveDamage�����
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{//���ٽ�ɫ
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{//���ѡ��һ����ʼ�����ɽ�ɫ
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
