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

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());//初始化角色
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)//类似于tick函数，每一帧frame都会调用
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


	//offset yaw for strafing（获取用于扫射的yaw偏移）
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();//是全局旋转,只是camera朝向的方向
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());//是角色运动时角色朝向的方向
	//UE_LOG(LogTemp, Warning, TEXT("MovementRotation Yaw %f: "), MovementRotation.Yaw);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);/** Normalized A-B */
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);//插值平滑
	YawOffset = DeltaRotation.Yaw;
	
	//用于lean，绕x轴旋转
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);//插值平滑
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();//角色类每帧更新AO_Yaw,这里再获取,在动画蓝图中使用
	AO_Pitch = BlasterCharacter->GetAO_Pitch();
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw : %f"), AO_Yaw);
	//设置FABRIK，调整左手持枪位置
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTranform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTranform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTranform.SetLocation(OutPosition);
		LeftHandTranform.SetRotation(FQuat(OutRotation));

		//修改右手rotation，使得枪和HitTarget方向一致
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));//RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget())
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);//平滑
		}
		
		bUseFABRIK = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
		bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetbDisableGameplay();
		bTransformRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetbDisableGameplay();

		//FVector A = BlasterCharacter->GetHitTarget();
		//FVector B = RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget());
		//UE_LOG(LogTemp, Warning, TEXT("HitTarget : %f %f %f"), A.X, A.Y, A.Z);
		//UE_LOG(LogTemp, Warning, TEXT("2 : %f %f %f"), B.X, B.Y, B.Z);
		
		////绘制枪口指向方向与子弹实际运动轨迹方向
		////GetSocketTransform函数用于获取骨骼中指定Socket的变换信息（位置、旋转、缩放）。
		/*FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 5000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);*/

	}
}
