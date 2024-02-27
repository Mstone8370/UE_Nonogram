// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI_Manager.generated.h"

UENUM(BlueprintType)
enum class EUIState : uint8
{
	EUS_None              UMETA(DisplayName = "None"),
	EUS_Main              UMETA(DisplayName = "Main"),
	EUS_FolderSelect      UMETA(DisplayName = "FolderSelect"),
	EUS_PuzzleSelect      UMETA(DisplayName = "PuzzleSelect"),
	EUS_UserPuzzle        UMETA(DisplayName = "UserPuzzle"),
	EUS_Play              UMETA(DisplayName = "Play"),
	EUS_PuzzleMaker       UMETA(DisplayName = "PuzzleMaker"),
	EUS_PuzzleCleared     UMETA(DisplayName = "PuzzleCleared"),
	EUS_Setting           UMETA(DisplayName = "Setting"),
	EUS_PuzzleSizeSelect  UMETA(DisplayName = "PuzzleSizeSelect"),

	EUS_MAX               UMETA(DisplayName = "MAX")
};

UCLASS( Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NONOGRAM_API UUI_Manager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUI_Manager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	TArray<EUIState> UI_Stack;
		
};
