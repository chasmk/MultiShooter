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
	void SetHUDAnnouncementCountdown(float CountdownTime);//设置announce界面的倒计时文本
	
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime(); //获取此时server上的时间
	/*Called after this PlayerController's viewport/net connection is associated with this player controller.*/
	//这是我们能在server上获得时间的最早的地方，所以我们在这里向server发送请求
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

protected:
	virtual void BeginPlay() override;
	/**
	 * @brief 设置HUD中的倒计时时间
	 */
	void SetHUDTime();
	void PollInit();
	
	/**
	 * Sync time 同步 client与server之间的time
	 */
	
	/**
	* @brief server上执行的RPC
	* 请求当前的server time，把client当前时间作为参数传递
	* @param TimeOfClientRequest 
	*/
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	
	/**
	 * @brief client上执行的RPC
	 * 向客户端报告当前的服务器时间 以响应ServerRequestServerTime
	 * @param TimeOfClientRequest client发送请求的时间
	 * @param TimeServerReceivedClientRequest server接收到该请求时的时间
	 */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	/**
	 * @brief client和server之间的时间差
	 */
	float ClientServerDelta = 0.f;

	/**
	 * @brief client与server之间多久同步一次时间
	 */
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	/**
	 * @brief 每隔TimeSyncFrequency的时间就同步一次time
	 * @param DeltaTime 
	 */
	void CheckTimeSync(float DeltaTime);

	/**
	 * @brief 从server上获取GameMode实例，并从中获取time相关的初值。
	 * 并在最后调用ClientJoinMidgame RPC 把这些值同步到client上
	 */
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();

	/**
	 * @brief 用于从server上同步time相关变量的值和MatchState到client上
	 * 该RPC最终会在所有机器上运行，并检查matchstate，若是WaitingToStart，则把announcement添加到HUD上
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
	 *当这个变量设置为true后，说明CharacterOverlay已经被set了，但这时CharacterOverlay可能并没有valid，所以需要再初始化一下
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
