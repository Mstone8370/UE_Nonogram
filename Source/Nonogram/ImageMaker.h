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

private:
	FString ImgFolderName;
	FString InProgressFolderName;

	FString SavedDir;
	FString ImgDir;
	FString InProgressDir;

	bool bIsImgDirValid;
	bool bIsInProgressDirValid;

	void CreateFolders();

	void Write2Byte(uint16 Value, uint8* File, int32 Offset) const;
	void Write4Byte(uint32 Value, uint8* File, int32 Offset) const;

	void DecodeAndFillPixelData(const FString& Data, uint8* FileData, int32 Color, uint32 FileSize, uint32 RowBytes) const;

	static const uint32 FILE_HEADER_SIZE;
	static const uint32 DIB_HEADER_SIZE;
	static const uint8  FILE_SIZE_OFFSET;
	static const uint8  OFF_BITS_OFFSET;
	static const uint32 COLOR_TABLE_SIZE;
	static const uint16 COLOR_PLANE;
	static const uint16 COLOR_DEPTH;
	static const uint8  EMPTY_COLOR;

public:
	bool SaveInProgressImage(const FString& Data, const FString& FolderName, const FString& BoardName, int32 RowSize, int32 ColSize, int32 Color = 255) const;
};