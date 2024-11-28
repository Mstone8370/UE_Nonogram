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

	// 마지막으로 작성한 오프셋의 다음 위치 리턴
	template <typename T>
	uint32 WriteBytes(T Value, uint8* File, uint32 Offset, uint32 ByteCount) const;

	uint32 Write2Bytes(uint16 Value, uint8* File, uint32 Offset) const;
	uint32 Write4Bytes(uint32 Value, uint8* File, uint32 Offset) const;

	void DecodeAndFillPixelData(const FString& Data, uint8* FileData, uint8 Color, uint32 FileSize, uint32 RowBytes) const;

	static const uint32 FILE_HEADER_SIZE;
	static const uint32 DIB_HEADER_SIZE;
	static const uint8  FILE_SIZE_OFFSET;
	static const uint8  OFF_BITS_OFFSET;
	static const uint32 COLOR_TABLE_SIZE;
	static const uint16 COLOR_PLANE;
	static const uint16 COLOR_DEPTH;
	static const uint8  EMPTY_COLOR;

public:
	bool SaveInProgressImage(const FString& Data, const FString& FolderName, const FString& BoardName, uint32 RowSize, uint32 ColSize, uint8 Color = 255) const;
};

template <typename T>
uint32 UImageMaker::WriteBytes(T Value, uint8* File, uint32 Offset, uint32 ByteCount) const
{
	for (uint32 i = 0; i < ByteCount; ++i)
	{
		File[Offset + i] = static_cast<uint8>((Value >> (8 * i)) & 0xFF);
	}
	return Offset + ByteCount;
}
