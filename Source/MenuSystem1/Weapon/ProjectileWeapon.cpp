// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;//ֻ����server��ִ��
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//��ȡǹ�� socket
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	if (MuzzleFlashSocket)
	{
		//��ȡǹ��socket��λ��
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// ��ǹ��muzzle flash socket �� traceUnderCrosshairsʮ���߹켣�µ�hit location 
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;//ָ������actor��������
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(//�����ӵ� actor
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
			}
		}
	}
}