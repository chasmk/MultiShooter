// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM1_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))//这行作用是把c++变量与同名的widget textblock关联起来
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* BuffBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;//得分数值
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;//死亡数值
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;//子弹数值
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;//携带的子弹数值
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponTypeText;//武器类型名字
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;//游戏倒数时间
};
