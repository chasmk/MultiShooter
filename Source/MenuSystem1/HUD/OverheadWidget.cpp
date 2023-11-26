// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//ENetRole LocalRole = InPawn->GetLocalRole(); //GetRemoteRole();
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString RRole;
	FString LRole;
	switch (RemoteRole)//看当前角色是哪一种网络角色
	{
	case ENetRole::ROLE_Authority:
		RRole = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		RRole = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		RRole = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		RRole = FString("None");
		break;
	}
	switch (LocalRole)//看当前角色是哪一种网络角色
	{
	case ENetRole::ROLE_Authority:
		LRole = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		LRole = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		LRole = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		LRole = FString("None");
		break;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *RRole);
	FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *LRole);
	SetDisplayText(RemoteRoleString + " " + LocalRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	APlayerState* PlayerState = InPawn->GetPlayerState();

	FString PlayerName;
	if (PlayerState)
	{
		PlayerName = PlayerState->GetPlayerName();
	}
	SetDisplayText(DisplayText->GetText().ToString() + FString("name: ") + PlayerName);

}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);//当我们过渡到或离开某个level时调用
}
/*getname的另一解法，from discord
void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	const APlayerState* PlayerState = InPawn->GetPlayerState();
	if (!PlayerState || !*PlayerState->GetPlayerName() && TotalTime < GetPlayerNameTimeout)
	{//当playerstate不存在  或者  state里面没有name且没有超时
		FTimerHandle GetPlayerStateTimer;
		FTimerDelegate TryAgainDelegate;
		TryAgainDelegate.BindUFunction(this, FName("ShowPlayerName"), InPawn);
		GetWorld()->GetTimerManager().SetTimer(GetPlayerStateTimer, TryAgainDelegate, GetPlayerNameInterval, false, 0.1f);
		TotalTime += GetPlayerNameInterval;
		return;
	}
	const FString PlayerName = InPawn->GetPlayerState()->GetPlayerName();
	SetDisplayText(PlayerName);
}*/
