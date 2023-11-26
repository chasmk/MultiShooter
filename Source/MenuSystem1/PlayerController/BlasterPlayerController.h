// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Styling/SlateColor.h"
#include "Math/Color.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM1_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ABlasterPlayerController();
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDBuff(float CurValue, float MaxValue);
	void SetHUDBuffVisibility(ESlateVisibility IsVisible);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponType(FString WeaponType);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);//����announce����ĵ���ʱ�ı�
	
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime(); //��ȡ��ʱserver�ϵ�ʱ��
	/*Called after this PlayerController's viewport/net connection is associated with this player controller.*/
	//������������server�ϻ��ʱ�������ĵط�������������������server��������
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

protected:
	virtual void BeginPlay() override;
	/**
	 * @brief ����HUD�еĵ���ʱʱ��
	 */
	void SetHUDTime();
	void PollInit();
	
	/**
	 * Sync time ͬ�� client��server֮���time
	 */
	
	/**
	* @brief server��ִ�е�RPC
	* ����ǰ��server time����client��ǰʱ����Ϊ��������
	* @param TimeOfClientRequest 
	*/
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	
	/**
	 * @brief client��ִ�е�RPC
	 * ��ͻ��˱��浱ǰ�ķ�����ʱ�� ����ӦServerRequestServerTime
	 * @param TimeOfClientRequest client���������ʱ��
	 * @param TimeServerReceivedClientRequest server���յ�������ʱ��ʱ��
	 */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	/**
	 * @brief client��server֮���ʱ���
	 */
	float ClientServerDelta = 0.f;

	/**
	 * @brief client��server֮����ͬ��һ��ʱ��
	 */
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	/**
	 * @brief ÿ��TimeSyncFrequency��ʱ���ͬ��һ��time
	 * @param DeltaTime 
	 */
	void CheckTimeSync(float DeltaTime);

	/**
	 * @brief ��server�ϻ�ȡGameModeʵ���������л�ȡtime��صĳ�ֵ��
	 * ����������ClientJoinMidgame RPC ����Щֵͬ����client��
	 */
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();

	/**
	 * @brief ���ڴ�server��ͬ��time��ر�����ֵ��MatchState��client��
	 * ��RPC���ջ������л��������У������matchstate������WaitingToStart�����announcement��ӵ�HUD��
	 */
	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;
	
	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	/**
	 *�������������Ϊtrue��˵��CharacterOverlay�Ѿ���set�ˣ�����ʱCharacterOverlay���ܲ�û��valid��������Ҫ�ٳ�ʼ��һ��
	 */
	//bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
	int32 HUDAmmo;
	bool bInitializeAmmo = false;
	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

};
