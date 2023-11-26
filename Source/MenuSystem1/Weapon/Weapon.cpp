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
	SetRootComponent(WeaponMesh);//设置weapon为root组件

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//ECR_Block：允许物理碰撞，并阻止物体穿透。
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//pawn可以穿过武器，忽略碰撞。此时是有碰撞的，只是不会响应
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//Actor的包围盒将不再与其他Actor进行任何碰撞或触发事件。

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//忽略碰撞响应，因为我们只在服务器上重叠时才响应
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//只在服务器上设置碰撞

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())//查看当前是否是服务器 GetLocalRole() == ENetRole::ROLE_Authority
	{//服务器将负责所有武器！！！
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//允许物体重叠，但不会产生物理碰撞。
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);//只有在服务器上才绑定委托
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);//最开始提示不可见，角色走进重叠时才可见
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
		BlasterCharacter->SetOverlappingWeapon(nullptr);//结束重叠时，把武器指针传入角色中
	}
}

void AWeapon::SetWeaponState(EWeaponState State)//只在server上调用，client只能RPC远程调用
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//ShowPickupWidget(false);
		//下面这行会自动调用OnSphereEndOverlap，然后武器提示text会自己消失，所以我们不需要再自行设置ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//只保留这一行就行，效果一样
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{//只在server上
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

void AWeapon::OnRep_WeaponState()//每次server上复制更新WeaponState变量，所有clients都会调用它
{								 //每次一个client远程调用后在server上复制更新WeaponState变量，所有client会调用它，除了client
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//ShowPickupWidget(false);//client拾取后不显示，但其它client和server上还显示
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
		{//前面一同操作都是为了获取Controller
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);//设置子弹数
		}
	}
}

void AWeapon::SpendRound()
{//只在server上call
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{//用于client的设置子弹数
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
		BlasterCharacter->SetOverlappingWeapon(this);//重叠时，把武器指针传入角色中
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
		WeaponMesh->PlayAnimation(FireAnimation, false);//播放开枪动画
	}

	//获取弹壳 socket
	const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
	if (AmmoEjectSocket)
	{
		//获取弹壳socket的位置
		FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
	
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<ACasing>(//生成弹壳 actor
				CasingClass,
				SocketTransform.GetLocation(),
				SocketTransform.GetRotation().Rotator()
				);
		}
	}
	SpendRound();//减少子弹数
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
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);//用clamp防止出现负数
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}
