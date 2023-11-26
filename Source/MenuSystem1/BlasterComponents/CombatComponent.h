// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MenuSystem1/HUD/BlasterHUD.h"
#include "MenuSystem1/Weapon/WeaponTypes.h"
#include "MenuSystem1/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MENUSYSTEM1_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;//声明为友元类，这样角色类就对此类有完全访问
	//父函数定义：
	// Function called every frame on this ActorComponent. Override this function to implement 
	// custom logic to be executed every frame.
	// Only executes if the component is registered, and also 
	// PrimaryComponentTick.bCanEverTick must be set to true.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/**
	 * 将武器装备到角色身上，并修改角色视角设置
	 */
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/**
	 * 角色类中press事件触发后调用该函数
	 */
	void FireButtonPressed(bool bPressed);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

protected:
	/**
	 * 设置角色移动速度和相机FOV值
	 */
	virtual void BeginPlay() override;

	/**
	 * 设置角色是否在瞄准，在角色类中调用
	 */
	void SetAiming(bool bIsAiming);
	
	/**
	 * 上面函数对应的RPC，客户端调用，服务器上执行 
	 */
	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	/**
	 * 这个onRep用来解决client的装备武器和自己上的leaning和strafing
	 */
	UFUNCTION()
		void OnRep_EquippedWeapon();

	/**
	 * 通过调用RPC，播放角色和枪的动画
	 */
	void Fire();

	UFUNCTION(Server, Reliable)//RPC
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)//multicast  RPC
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/**
	 * 射线追踪，获取射线hit的对象，若瞄准人，十字线设置为红色
	 */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/**
	 * 设置准心的位置与缩放
	 */
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();//计算换弹时需要补充多少子弹

private:
	UPROPERTY()
	class ABlasterCharacter* Character;//这里的class是前向声明，可以先声明，后定义
	UPROPERTY()
	class ABlasterPlayerController* Controller;//用于获取HUD
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	//用于设置瞄准前后角色的不同移动速度
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	//FVector HitTarget;//弃用，

	/** 
	* HUD and crosshairs
	*/
	float CrosshairVelocityFactor;//奔跑时
	float CrosshairInAirFactor;//在空中时
	float CrosshairAimFactor;//瞄准时
	float CrosshairShootingFactor;//射击时

	FVector HitTarget;//弃用，再用

	FHUDPackage HUDPackage;

	/**
	* Aiming and FOV 通过调整FOV实现瞄准效果
	*/

	// 没有瞄准时的field of views 在BeginPlay中设置camera的base FOV
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	/**
	 * 平滑过渡视野，在TickComponent中调用
	 */
	void InterpFOV(float DeltaTime);//

	/**
	* Automatic fire 自动开火
	*/
	FTimerHandle FireTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	bool bCanFire = true;

	/**
	 * 设置自动开火定时器
	 */
	void StartFireTimer();
	/**
	 * 每次定时器触发都会调用这个函数，函数会执行一次Fire
	 */
	void FireTimerFinished();

	bool CanFire();

	// 当前装备武器的携带弹药数
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	void UpdateCarriedAmmo();//单纯更新HUD中的CarriedAmmo
public:	
	
		
};
