// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PuzzleDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class NONOGRAM_API UPuzzleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UTexture* Tex;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UTexture2D* Tex2D;
};
