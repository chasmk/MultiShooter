// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()//关联反射系统
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;//用于动态 准心十字线
	FLinearColor CrosshairsColor;// 准心颜色
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
	virtual void DrawHUD() override;//每个frame都调用
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;//在蓝图里设置

	/**
	 * @brief 把CharacterOverlay添加到视口中
	 */
	void AddCharacterOverlay();//把血条添加到viewport

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	
	UPROPERTY()
	class UAnnouncement* Announcement;

	/**
	 * @brief 把Announcement添加到视口中
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
	// 在combat component中调用
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
