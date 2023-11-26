// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "MenuSystem1/BlasterComponents/CombatComponent.h"
#include "MenuSystem1/Character/BlasterCharacter.h"


AAmmoPickup::AAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		UCombatComponent* Combat = BlasterCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}