// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageMaker.h"

UImageMaker::UImageMaker()
    :
    ImgFolderName(TEXT("img")),
    InProgressFolderName(TEXT("InProgress")),
    bIsImgDirValid(false),
    bIsInProgressDirValid(false)
{
    SavedDir = FPaths::ProjectSavedDir();
    
    ImgDir = SavedDir;
    ImgDir.PathAppend(*ImgFolderName, ImgFolderName.Len());
    InProgressDir = ImgDir;
    InProgressDir.PathAppend(*InProgressFolderName, InProgressFolderName.Len());

    CreateFolders();
}

void UImageMaker::CreateFolders()
{
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
    
    bIsImgDirValid = FileManager.CreateDirectory(*ImgDir);
    bIsInProgressDirValid = FileManager.CreateDirectory(*InProgressDir);
}

void UImageMaker::Write2Byte(uint16 Value, uint8* File, int32 Offset) const
{
    File[Offset] = uint8(Value);
    File[Offset + 1] = uint8(Value >> 8);
}

void UImageMaker::Write4Byte(uint32 Value, uint8* File, int32 Offset) const
{
    Write2Byte(uint16(Value), File, Offset);
    Write2Byte(uint16(Value >> 16), File, Offset + 2);
}

bool UImageMaker::SaveInProgressImage(FString& Data, FString& FolderName, FString& BoardName, int32 RowSize, int32 ColSize, int32 Color) const
{
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

    FString FileDir = InProgressDir;
    FileDir.PathAppend(*FolderName, FolderName.Len());
    FileManager.CreateDirectory(*FileDir);
    FileDir.PathAppend(*BoardName, BoardName.Len());
    FileDir.Append(".bmp");

    uint32 Width = ColSize;
    uint32 Height = RowSize;
    // Bitmap의 픽셀 정보에서 가로 줄의 바이트는 4의 배수로 만들어야 함.
    uint32 HorizontalLength = FMath::CeilToInt32(Width / 4.f) * 4;

    // Bitmap file header size + DIB header size + Color table size = 14 + 40 + 256 * 4 = 1078
    uint32 PixelStorageOffset = 14 + 40 + 256 * 4;
    uint32 FileSize = PixelStorageOffset + HorizontalLength * Height;

    uint8* FileData = new uint8[FileSize];
    memset(FileData, 0, sizeof(uint8) * FileSize);

    // Bitmap file header
    FileData[0] = 'B';
    FileData[1] = 'M';
    Write4Byte(FileSize, FileData, 2);             // File size
    Write4Byte(PixelStorageOffset, FileData, 10);  // Pixel storage offset

    // DIB header
    Write4Byte(40, FileData, 14);                  // DIB header size
    Write4Byte(Width, FileData, 18);               // Width
    Write4Byte(Height, FileData, 22);              // Hegith
    Write2Byte(1, FileData, 26);                   // 1 color plane
    Write2Byte(8, FileData, 28);                   // 8 bit color

    // Color table
    // 00 00 00 00 ~ FF FF FF 00
    for (int32 ColorTable = 0; ColorTable < 256; ColorTable++)
    {
        // 54 = Color table offset
        FileData[54 + ColorTable * 4 + 0] = ColorTable;  // B
        FileData[54 + ColorTable * 4 + 1] = ColorTable;  // G
        FileData[54 + ColorTable * 4 + 2] = ColorTable;  // R
    }

    // Pixel storage
    int32 x = 0;
    int32 y = 0;

    uint8 FilledColor = (uint8)Color;
    uint8 EmptyColor = 255;
    uint8 CurrentColor = EmptyColor;
    for (const TCHAR& c : Data)
    {
        if (c == '|')
        {
            x = 0;
            y++;
            continue;
        }

        if ('0' <= c && c < '4')
        {
            CurrentColor = (c == '1') ? FilledColor : EmptyColor;
        }
        else
        {
            const int32 Length = UNonogramStatics::CharToNum(c);
            for (int32 i = 0; i < Length; i++)
            {
                int32 Offset = FileSize - ((y + 1) * HorizontalLength) + x;
                FileData[Offset] = CurrentColor;
                x++;
            }
        }
    }

    bool bSuccess = false;
    IFileHandle* FileHandle = FileManager.OpenWrite(*FileDir, false, true);
    if (FileHandle)
    {
        bSuccess = FileHandle->Write(FileData, sizeof(uint8) * FileSize);
        delete FileHandle;  // 파일을 닫기 위해 IFileHandle을 delete
    }
    delete[] FileData;
    return bSuccess;
}
