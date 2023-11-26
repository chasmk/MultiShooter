// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"

void UCharacterOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay initialization done"));
}
