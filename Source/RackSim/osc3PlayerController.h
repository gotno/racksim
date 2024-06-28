// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "osc3PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RACKSIM_API Aosc3PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    Aosc3PlayerController();
protected:
    virtual void BeginPlay() override;
};