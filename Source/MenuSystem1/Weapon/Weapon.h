// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)//��ͼ��Ҳ�������ö�����ͣ�����״̬
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DiaplayName = "Initial State"),
	EWS_Equipped UMETA(DiaplayName = "Equipped State"),
	EWS_Dropped UMETA(DiaplayName = "Dropped State"),

	EWS_MAX UMETA(DiaplayName = "DefaultMax")//��ʾ���ö�����ж��ٸ�����
};

UCLASS()
class MENUSYSTEM1_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ע����Щ������Ҫ������ͬ���б�����
	virtual void OnRep_Owner() override;//��actor��owner�仯ʱ�ͻ����
	void SetHUDAmmo();//����HUD�е�ǰ���е��ӵ���
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();//�����ӵ�ԭ��
	void AddAmmo(int32 AmmoToAdd);

	/**
	* ����׼�ĵ�Texture 
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairsBottom;

	/**
	*  Zoomed FOV while aiming ��׼ʱ��Ұ����
	*/

	UPROPERTY(EditAnywhere)
		float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	/**
	* Automatic Fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = Combat)
		bool bAutomatic = true;

	//��ǹʱ������
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedCompnent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);//���������ײ����

	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedCompnent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* WeaponMesh;//ǹ��mesh

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;//������ײ����

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;//����״̬��һ��ö�����ͣ���������Ϊ�ɸ��ƣ�

	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;//ʰȡ������������ʾ

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;//ǹ���𶯻���

	UPROPERTY(EditAnywhere)
		TSubclassOf<class ACasing> CasingClass;

	/**
	 * ��ҩ����ֻ��server���޸�
	 */
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	/**
	 * ��ȥ�ӵ������������û��owner
	 */
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();//��ǰ�����Ƿ�Ϊ��
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
