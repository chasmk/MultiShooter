// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()//��������ϵͳ
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;//���ڶ�̬ ׼��ʮ����
	FLinearColor CrosshairsColor;// ׼����ɫ
};

/**
 * 
 */
UCLASS()
class MENUSYSTEM1_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABlasterHUD();
	virtual void Tick(float DeltaSeconds) override;
	virtual void DrawHUD() override;//ÿ��frame������
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;//����ͼ������

	/**
	 * @brief ��CharacterOverlay��ӵ��ӿ���
	 */
	void AddCharacterOverlay();//��Ѫ����ӵ�viewport

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	
	UPROPERTY()
	class UAnnouncement* Announcement;

	/**
	 * @brief ��Announcement��ӵ��ӿ���
	 */
	void AddAnnouncement();
protected:
	virtual void BeginPlay() override;
	
private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	// ��combat component�е���
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
