// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageMaker.h"
#include "Misc/Paths.h"

const uint32 UImageMaker::FILE_HEADER_SIZE = 14;
const uint32 UImageMaker::DIB_HEADER_SIZE  = 40;
const uint8  UImageMaker::FILE_SIZE_OFFSET = 2;
const uint8  UImageMaker::OFF_BITS_OFFSET  = 10;
const uint32 UImageMaker::COLOR_TABLE_SIZE = 256 * 4;
const uint16 UImageMaker::COLOR_PLANE      = 1;
const uint16 UImageMaker::COLOR_DEPTH      = 8;
const uint8  UImageMaker::EMPTY_COLOR      = 255;

UImageMaker::UImageMaker()
    : ImgFolderName(TEXT("img"))
    , InProgressFolderName(TEXT("InProgress"))
    , SavedDir(FPaths::ProjectSavedDir())
    , ImgDir(FPaths::Combine(SavedDir, ImgFolderName))
    , InProgressDir(FPaths::Combine(ImgDir, InProgressFolderName))
    , bIsImgDirValid(false)
    , bIsInProgressDirValid(false)
{
    CreateFolders();
}

void UImageMaker::CreateFolders()
{
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
    
    bIsImgDirValid = FileManager.CreateDirectory(*ImgDir);
    bIsInProgressDirValid = FileManager.CreateDirectory(*InProgressDir);
}

uint32 UImageMaker::Write2Bytes(uint16 Value, uint8* File, uint32 Offset) const
{
    return WriteBytes(Value, File, Offset, 2);
}

uint32 UImageMaker::Write4Bytes(uint32 Value, uint8* File, uint32 Offset) const
{
    return WriteBytes(Value, File, Offset, 4);
}

void UImageMaker::DecodeAndFillPixelData(const FString& Data, uint8* FileData, uint8 Color, uint32 FileSize, uint32 RowBytes) const
{
    uint8 CurrentColor = EMPTY_COLOR;

    int32 x = 0;
    int32 y = 0;
    for (const TCHAR& c : Data)
    {
        if (c == '|')
        {
            x = 0;
            ++y;
            continue;
        }

        if ('0' <= c && c < '4')
        {
            CurrentColor = (c == '1') ? Color : EMPTY_COLOR;
        }
        else
        {
            const int32 Length = UNonogramStatics::CharToNum(c);
            for (int32 i = 0; i < Length; i++)
            {
                // 아래에서부터 위로 픽셀 저장
                int32 Offset = FileSize - ((y + 1) * RowBytes) + x;
                FileData[Offset] = CurrentColor;
                ++x;
            }
        }
    }
}

bool UImageMaker::SaveInProgressImage(const FString& Data, const FString& FolderName, const FString& BoardName, uint32 RowSize, uint32 ColSize, uint8 Color) const
{
    // 파일 시스템 접근을 위한 파일 매니저
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    // 파일 경로
    const FString FolderDir = FPaths::Combine(InProgressDir, FolderName);
    const FString FileName = BoardName + ".bmp";
    const FString FileDir = FPaths::Combine(FolderDir, BoardName + ".bmp");
    
    // 폴더 생성
    FileManager.CreateDirectory(*FolderDir);

    // 이미지 크기 및 파일 크기 계산
    const uint32 Width = ColSize;
    const uint32 Height = RowSize;
    const uint32 RowBytes = FMath::CeilToInt32(Width / 4.f) * 4; // 4의 배수로
    const uint32 PixelStorageOffset = FILE_HEADER_SIZE + DIB_HEADER_SIZE + COLOR_TABLE_SIZE;
    const uint32 FileSize = PixelStorageOffset + (RowBytes * Height);

    // 파일 데이터 메모리 할당
    TUniquePtr<uint8[]> FileData = MakeUnique<uint8[]>(FileSize);
    FMemory::Memzero(FileData.Get(), FileSize);

    // Bitmap file header
    FileData[0] = 'B';
    FileData[1] = 'M';
    Write4Bytes(FileSize, FileData.Get(), FILE_SIZE_OFFSET);           // File size
    Write4Bytes(PixelStorageOffset, FileData.Get(), OFF_BITS_OFFSET);  // Pixel storage offset

    // DIB header
    {
        uint32 CurrentOffset = FILE_HEADER_SIZE;
        CurrentOffset = Write4Bytes(DIB_HEADER_SIZE, FileData.Get(), CurrentOffset);    // DIB header size
        CurrentOffset = Write4Bytes(Width, FileData.Get(), CurrentOffset);              // Width
        CurrentOffset = Write4Bytes(Height, FileData.Get(), CurrentOffset);             // Hegith
        CurrentOffset = Write2Bytes(COLOR_PLANE, FileData.Get(), CurrentOffset);        // 1 color plane
        CurrentOffset = Write2Bytes(COLOR_DEPTH, FileData.Get(), CurrentOffset);        // 8 bit color
    }

    // Color table
    // 00 00 00 00 ~ FF FF FF 00 (Gray Scale)
    for (int32 ColorIdx = 0; ColorIdx < 256; ColorIdx++)
    {
        const SIZE_T ColorTableOffset = FILE_HEADER_SIZE + DIB_HEADER_SIZE + (ColorIdx * 4);
        FileData[ColorTableOffset + 0] = ColorIdx;  // Blue
        FileData[ColorTableOffset + 1] = ColorIdx;  // Green
        FileData[ColorTableOffset + 2] = ColorIdx;  // Red
    }

    // Pixel storage
    DecodeAndFillPixelData(Data, FileData.Get(), Color, FileSize, RowBytes);

    // Save image file
    bool bSuccess = false;
    IFileHandle* FileHandle = FileManager.OpenWrite(*FileDir, false, true);
    if (FileHandle)
    {
        bSuccess = FileHandle->Write(FileData.Get(), sizeof(uint8) * FileSize);
        delete FileHandle;  // 파일을 닫기 위해 IFileHandle을 delete
    }

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save image: %s"), *FileDir);
    }
    return bSuccess;
}
