// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NonogramStructs.h"
#include "ImageMaker.generated.h"

/**
 * 
 */

UCLASS(BlueprintType, Blueprintable)
class NONOGRAM_API UImageMaker : public UObject
{
	GENERATED_BODY()

public:
	UImageMaker();

protected:
	FString SavedDir;
	FString ImgDir;
	FString InProgressDir;

	FString ImgFolderName;
	FString InProgressFolderName;

	bool bIsImgDirValid;
	bool bIsInProgressDirValid;

	void CreateFolders();
	void Write2Byte(uint16 Value, uint8* File, int32 Offset) const;
	void Write4Byte(uint32 Value, uint8* File, int32 Offset) const;

public:
	bool SaveInProgressImage(FString& Data, FString& FolderName, FString& BoardName, int32 RowSize, int32 ColSize, int32 Color = 255) const;
};