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
	
	UPROPERTY(meta = (BindWidget))//���������ǰ�c++������ͬ����widget textblock��������
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* BuffBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;//�÷���ֵ
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;//������ֵ
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;//�ӵ���ֵ
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;//Я�����ӵ���ֵ
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponTypeText;//������������
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;//��Ϸ����ʱ��
};
