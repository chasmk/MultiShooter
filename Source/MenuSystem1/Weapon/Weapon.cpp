// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MenuSystem1/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MenuSystem1/PlayerController/BlasterPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;//If true, this actor will replicate to remote machines

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);//����weaponΪroot���

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//ECR_Block������������ײ������ֹ���崩͸��
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//pawn���Դ���������������ײ����ʱ������ײ�ģ�ֻ�ǲ�����Ӧ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//Actor�İ�Χ�н�����������Actor�����κ���ײ�򴥷��¼���

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//������ײ��Ӧ����Ϊ����ֻ�ڷ��������ص�ʱ����Ӧ
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//ֻ�ڷ�������������ײ

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())//�鿴��ǰ�Ƿ��Ƿ����� GetLocalRole() == ENetRole::ROLE_Authority
	{//��������������������������
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//���������ص������������������ײ��
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);//ֻ���ڷ������ϲŰ�ί��
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);//�ʼ��ʾ���ɼ�����ɫ�߽��ص�ʱ�ſɼ�
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedCompnent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && PickupWidget)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);//�����ص�ʱ��������ָ�봫���ɫ��
	}
}

void AWeapon::SetWeaponState(EWeaponState State)//ֻ��server�ϵ��ã�clientֻ��RPCԶ�̵���
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//ShowPickupWidget(false);
		//�������л��Զ�����OnSphereEndOverlap��Ȼ��������ʾtext���Լ���ʧ���������ǲ���Ҫ����������ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//ֻ������һ�о��У�Ч��һ��
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{//ֻ��server��
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}

	/*if (GetOwner())
	{
		ENetRole remote = GetOwner()->GetRemoteRole();
		ENetRole local = GetOwner()->GetLocalRole();
		FString Remote = "";
		FString Local = "";
		switch (local)
		{
		case ENetRole::ROLE_Authority:Local = "Authority"; break;
		case ENetRole::ROLE_AutonomousProxy:Local = "AutonomousProxy"; break;
		case ENetRole::ROLE_SimulatedProxy:Local = "SimulatedProxy"; break;
		}
		switch (remote)
		{
		case ENetRole::ROLE_Authority:Remote = "Authority"; break;
		case ENetRole::ROLE_AutonomousProxy:Remote = "AutonomousProxy"; break;
		case ENetRole::ROLE_SimulatedProxy:Remote = "SimulatedProxy"; break;
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				300.f,
				FColor::Yellow,
				FString::Printf(TEXT("SetWeaponState	Local: %s		Remote: %s"), *Local, *Remote)
			);
		}
	}*/
}

void AWeapon::OnRep_WeaponState()//ÿ��server�ϸ��Ƹ���WeaponState����������clients���������
{								 //ÿ��һ��clientԶ�̵��ú���server�ϸ��Ƹ���WeaponState����������client�������������client
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//ShowPickupWidget(false);//clientʰȡ����ʾ��������client��server�ϻ���ʾ
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}

	/*if (GetOwner())
	{
		ENetRole remote = GetOwner()->GetRemoteRole();
		ENetRole local = GetOwner()->GetLocalRole();
		FString Remote;
		FString Local;
		switch(local)
		{
		case ENetRole::ROLE_Authority:Local = "Authority"; break;
		case ENetRole::ROLE_AutonomousProxy:Local = "AutonomousProxy"; break;
		case ENetRole::ROLE_SimulatedProxy:Local = "SimulatedProxy"; break;
		}
		switch (remote)
		{
		case ENetRole::ROLE_Authority:Remote = "Authority"; break;
		case ENetRole::ROLE_AutonomousProxy:Remote = "AutonomousProxy"; break;
		case ENetRole::ROLE_SimulatedProxy:Remote = "SimulatedProxy"; break;
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				300.f,
				FColor::Red,
				FString::Printf(TEXT("OnRep_WeaponState		Local: %s		Remote: %s"), *Local, *Remote)
			);
		}
	}*/
	
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{//ǰ��һͬ��������Ϊ�˻�ȡController
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);//�����ӵ���
		}
	}
}

void AWeapon::SpendRound()
{//ֻ��server��call
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{//����client�������ӵ���
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedCompnent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter && PickupWidget)
	{
		//PickupWidget->SetVisibility(true);
		BlasterCharacter->SetOverlappingWeapon(this);//�ص�ʱ��������ָ�봫���ɫ��
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);//���ſ�ǹ����
	}

	//��ȡ���� socket
	const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
	if (AmmoEjectSocket)
	{
		//��ȡ����socket��λ��
		FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
	
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<ACasing>(//���ɵ��� actor
				CasingClass,
				SocketTransform.GetLocation(),
				SocketTransform.GetRotation().Rotator()
				);
		}
	}
	SpendRound();//�����ӵ���
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);//��clamp��ֹ���ָ���
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}
