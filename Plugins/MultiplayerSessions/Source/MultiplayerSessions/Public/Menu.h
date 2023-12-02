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
	UFUNCTION(BlueprintCallable) //��ʼ���˵�����ͼ�е���
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAlljyj")),
	               FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:
	virtual bool Initialize() override; //��������ť��callback�����󶨵�button����
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override; //�����Ự����lobby���Ƴ��ؿ�
	//
	//callbacks ����MultiplayerSessionSubsystem���Զ����delegates
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
	UPROPERTY(meta = (BindWidget)) //����������widget��������ͬ��ʵ��button��
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	//button ��callback���������º󴥷�
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown(); //�Ƴ�menu

	// subsystemָ��,����handle����online session ����
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	//�������������Ĭ��ֵ
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAlljyj")};
	FString PathToLobby{TEXT("")};
	//�������ڲ���startsession���������Ŀ����menu����С������ͼ�ﲻ�ܵ���
	/*
	protected:
		UFUNCTION(BlueprintCallable)
			void StartGameSession();
			*/
};
