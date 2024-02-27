// Copyright Epic Games, Inc. All Rights Reserved.


#include "NonogramGameModeBase.h"
#include "BoardManager.h"

ANonogramGameModeBase::ANonogramGameModeBase():
    ClickMode(EClickMode::ECM_Fill)
{}

void ANonogramGameModeBase::BeginPlay()
{
    Super::BeginPlay();

}

void ANonogramGameModeBase::RequestUserPuzzleList(FString ExtraURL)
{
    FHttpModule& Module = FHttpModule::Get();
    FHttpRequestRef Request = Module.CreateRequest();
    Request->SetVerb(TEXT("GET"));
    FString BaseURL;
#if PLATFORM_ANDROID
    BaseURL = "http://mstone8370.ddns.net:5490/userpuzzle/list/";
#else
    BaseURL = "localhost:8000/userpuzzle/list/";
#endif
    Request->SetURL(BaseURL.Append(ExtraURL));
    Request->OnProcessRequestComplete().BindUObject(this, &ANonogramGameModeBase::OnUserPuzzleListResponseReceived);
    Request->ProcessRequest();
}

void ANonogramGameModeBase::RequestValidation(FString Body)
{
    FHttpModule& Module = FHttpModule::Get();
    FHttpRequestRef Request = Module.CreateRequest();
    Request->SetVerb(TEXT("POST"));
    FString BaseURL;
#if PLATFORM_ANDROID
    BaseURL = "http://mstone8370.ddns.net:5490/validation/";
#else
    BaseURL = "localhost:8000/validation/";
#endif
    Request->SetURL(BaseURL);
    Request->SetHeader("Content-Type", "text/html");
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindUObject(this, &ANonogramGameModeBase::OnValidationResponseReceived);
    Request->ProcessRequest();
}

void ANonogramGameModeBase::RequestUpload(FString Body)
{
    FHttpModule& Module = FHttpModule::Get();
    FHttpRequestRef Request = Module.CreateRequest();
    Request->SetVerb(TEXT("POST"));
    FString BaseURL;
#if PLATFORM_ANDROID
    BaseURL = "http://mstone8370.ddns.net:5490/userpuzzle/";
#else
    BaseURL = "localhost:8000/userpuzzle/";
#endif
    Request->SetURL(BaseURL);
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindUObject(this, &ANonogramGameModeBase::OnUploadResponseReceived);
    Request->ProcessRequest();
}

void ANonogramGameModeBase::OnUserPuzzleListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString Body = Response->GetContentAsString();
    UserPuzzleListResponseReceived.Broadcast(bWasSuccessful, Body);
}

void ANonogramGameModeBase::OnValidationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString Body = Response->GetContentAsString();
    ValidationResponseReceived.Broadcast(bWasSuccessful, Body);
}

void ANonogramGameModeBase::OnUploadResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString Body = Response->GetContentAsString();
    UploadResponseReceived.Broadcast(bWasSuccessful, Body);
}
