// Fill out your copyright notice in the Description page of Project Settings.


#include "UserPuzzleItem.h"

#include "Blueprint/AsyncTaskDownloadImage.h"

void UUserPuzzleItem::DownloadPuzzleImage(FString ExtraURL)
{
    FString BaseURL;
#if PLATFORM_ANDROID
    BaseURL = "http://mstone8370.ddns.net:5490";
#else
    BaseURL = "localhost:8000";
#endif
    UAsyncTaskDownloadImage* AsyncTask = UAsyncTaskDownloadImage::DownloadImage(BaseURL.Append(ExtraURL));
    AsyncTask->OnSuccess.AddDynamic(this, &UUserPuzzleItem::OnImageDownloaded);
    AsyncTask->OnFail.AddDynamic(this, &UUserPuzzleItem::OnImageDownloadFailed);
}

void UUserPuzzleItem::OnImageDownloaded(UTexture2DDynamic* Texture)
{
    PuzzleImage = Texture;
    PuzzleImageDownloaded.Broadcast(true, Texture);
}

void UUserPuzzleItem::OnImageDownloadFailed(UTexture2DDynamic* Texture)
{
    PuzzleImageDownloaded.Broadcast(false, nullptr);
}