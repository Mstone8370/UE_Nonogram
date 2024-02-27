// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NonogramStructs.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "NonogramGameModeBase.generated.h"

/**
 * 
 */

class UBoardManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUserPuzzleListResponseReceivedSignature, bool, bSuccess, FString, Body);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FValidationResponseReceivedSignature, bool, bSuccess, FString, Body);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUploadResponseReceivedSignature, bool, bSuccess, FString, Body);

UCLASS()
class NONOGRAM_API ANonogramGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ANonogramGameModeBase();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EClickMode ClickMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoardManager> BoardManager_Ref;

	UPROPERTY(BlueprintAssignable)
	FUserPuzzleListResponseReceivedSignature UserPuzzleListResponseReceived;
	UPROPERTY(BlueprintAssignable)
	FValidationResponseReceivedSignature ValidationResponseReceived;
	UPROPERTY(BlueprintAssignable)
	FUploadResponseReceivedSignature UploadResponseReceived;

	UFUNCTION(BlueprintCallable)
	void RequestUserPuzzleList(FString ExtraURL);
	UFUNCTION(BlueprintCallable)
	void RequestValidation(FString Body);
	UFUNCTION(BlueprintCallable)
	void RequestUpload(FString Body);

	void OnUserPuzzleListResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnValidationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnUploadResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
