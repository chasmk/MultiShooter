// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable) //初始化菜单，蓝图中调用
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAlljyj")),
	               FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:
	virtual bool Initialize() override; //这两个按钮的callback函数绑定到button里面
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override; //创建会话进入lobby后移除关卡
	//
	//callbacks 用于MultiplayerSessionSubsystem中自定义的delegates
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget)) //变量将会与widget中名字相同的实际button绑定
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	//button 的callback函数，按下后触发
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown(); //移除menu

	// subsystem指针,用于handle所有online session 函数
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//定义参数变量和默认值
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAlljyj")};
	FString PathToLobby{TEXT("")};
	//以下用于测试startsession，这个函数目标是menu，在小白人蓝图里不能调用
	/*
	protected:
		UFUNCTION(BlueprintCallable)
			void StartGameSession();
			*/
};
