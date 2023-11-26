// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class MENUSYSTEM1_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;//����mesh
	UPROPERTY(EditAnywhere)
		float  ShellEjectionImpulse;//

	UPROPERTY(EditAnywhere)
		class USoundCue* ShellSound;//��������

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	

};
