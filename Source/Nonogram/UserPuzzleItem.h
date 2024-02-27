// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UserPuzzleItem.generated.h"

/**
 * 
 */

class UTexture2DDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPuzzleImageDownloadedSignature, bool, bSuccess, UTexture2DDynamic*, TextureDynamic);

UCLASS(BlueprintType, Blueprintable)
class NONOGRAM_API UUserPuzzleItem : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	UTexture2DDynamic* PuzzleImage;

	UPROPERTY(BlueprintAssignable)
	FPuzzleImageDownloadedSignature PuzzleImageDownloaded;

	UFUNCTION(BlueprintCallable)
	void DownloadPuzzleImage(FString ExtraURL);

	UFUNCTION()
	void OnImageDownloaded(UTexture2DDynamic* Texture);
	UFUNCTION()
	void OnImageDownloadFailed(UTexture2DDynamic* Texture);
};
