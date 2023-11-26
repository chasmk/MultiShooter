// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MenuSystem1/BlasterTypes/TurningInPlace.h"
#include "MenuSystem1/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "MenuSystem1/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class MENUSYSTEM1_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//用来注册哪些变量需要在网络同步中被复制
	virtual void PostInitializeComponents() override;//用来初始化组件（组件已经把角色类设为友元类）
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();

	void Elim();//只在server上调用
	UFUNCTION(NetMulticast, Reliable)//作为RPC，同步死亡后动画
	void MulticastElim();//处理角色死亡后的事情

	//UFUNCTION(NetMulticast, Unreliable)
	//	void MulticastHit();//RPC 同步被击中 动画

	virtual void OnRep_ReplicatedMovement() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;//是否禁止玩家actions
	
	void UpdateHUDHealth();//更新血条
	void UpdateHUDShield();//更新蓝条
	
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);//W S
	void MoveRight(float Value);//A D
	void Turn(float Value);//鼠标x轴移动
	void LookUp(float Value);//鼠标y轴移动
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void CrouchButtonReleased();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();//解决角色转体卡顿
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();

	UFUNCTION()
		void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	
	void PollInit();//poll轮询所有相关类并初始化HUD
	void RotateInPlace(float DeltaTime);
	virtual void Destroyed() override;

private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;//红色射线boom

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;//跟拍相机camera

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;//头顶名字的widget

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)//让他可复制，只要变量改变，就会复制
		//这里我们只在server上修改它的值，所以它的值会从server同步到所有clients）
	class AWeapon* OverlappingWeapon;//正在重叠的武器weapon

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);//不会被server调用，它只会在变量被复制时被自动调用，
	//这里变量只会从server到client复制。所以它永远不会被复制到server，server也永远不会调用这个函数，
	//所以需要下面的函数解决server上的显示问题

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UFUNCTION(Server, Reliable)//reliable可以让server返回确认，若client没收到，会再次RPC
		void ServerEquipButtonPressed();//client调用，server执行

	//以下用于aim offset
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;//

	ETurningInPlace TurningInPlace;//原地转视角时可以转身
	void TurnInPlace(float DeltaTime);

	/**
	 * Animation montages
	 */

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;


	void HideCameraIfCharacterClose();//摄像头太近时就隐藏角色
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	* Player Health
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnyWhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	* Player Shield
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	/**
	 * Dissolve Effect
	 */

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;//delegate

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;//在蓝图中设置

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//Dynamic instance that 我们可以在runtime时改变它
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance 在蓝图里设置， 与Dynamic instance一起使用
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool ISElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetbDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE ABlasterPlayerController* GetPlayerController() const { return BlasterPlayerController;}
};
