// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MenuSystem1/Weapon/Weapon.h"
#include "DrawDebugHelpers.h"
#include "MenuSystem1/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());//��ʼ����ɫ
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)//������tick������ÿһ֡frame�������
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();//Returns velocity (in cm/s (Unreal Units/second) 
	Velocity.Z = 0.f;
	Speed = Velocity.Size();//Sqrt(X*X + Y*Y + Z*Z);

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->ISElimmed();
	//UE_LOG(LogTemp, Warning, TEXT("bIsAccelerating  %d: "), bIsAccelerating);


	//offset yaw for strafing����ȡ����ɨ���yawƫ�ƣ�
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();//��ȫ����ת,ֻ��camera����ķ���
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());//�ǽ�ɫ�˶�ʱ��ɫ����ķ���
	//UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f: "), MovementRotation.Yaw);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);/** Normalized A-B */
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);//��ֵƽ��
	YawOffset = DeltaRotation.Yaw;
	
	//����lean����x����ת
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);//��ֵƽ��
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();//��ɫ��ÿ֡����AO_Yaw,�����ٻ�ȡ,�ڶ�����ͼ��ʹ��
	AO_Pitch = BlasterCharacter->GetAO_Pitch();
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw : %f"), AO_Yaw);
	//����FABRIK���������ֳ�ǹλ��
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTranform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTranform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTranform.SetLocation(OutPosition);
		LeftHandTranform.SetRotation(FQuat(OutRotation));

		//�޸�����rotation��ʹ��ǹ��HitTarget����һ��
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));//RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget())
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);//ƽ��
		}
		
		bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
		bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetbDisableGameplay();
		bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetbDisableGameplay();

		//FVector A = BlasterCharacter->GetHitTarget();
		//FVector B = RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget());
		//UE_LOG(LogTemp, Warning, TEXT("HitTarget : %f %f %f"), A.X, A.Y, A.Z);
		//UE_LOG(LogTemp, Warning, TEXT("2 : %f %f %f"), B.X, B.Y, B.Z);
		
		////����ǹ��ָ�������ӵ�ʵ���˶��켣����
		////GetSocketTransform�������ڻ�ȡ������ָ��Socket�ı任��Ϣ��λ�á���ת�����ţ���
		/*FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 5000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);*/

	}
}
