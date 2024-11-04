// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NonogramStructs.h"
#include "BoardManager.generated.h"

#define IN_RANGE(n, left, right) (left <= n && n < right)

/**
 * 
 */

class ANonogramGameModeBase;
class UPlayLogger;
class UImageMaker;

UENUM(BlueprintType)
enum class EBoardMode : uint8
{
	EBM_Play UMETA(DisplayName = "Play"),
	EBM_Make UMETA(DisplayName = "Make"),
	EBM_User UMETA(DisplayName = "UserPuzzle"),

	EBM_None UMETA(DisplayName = "None"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBoardClearedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNewBoardSetSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRowLineCheckUpdatedSignature, int32, RowIdx, FLineCheck, LineCheck);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FColLineCheckUpdatedSignature, int32, ColIdx, FLineCheck, LineCheck);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCellStateChangedSignature, int32, X, int32, Y, ECellState, NewCellState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRowInfoUpdatedSignature, int32, RowIdx, FLineInfo, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FColInfoUpdatedSignature, int32, ColIdx, FLineInfo, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUndoButtonEnableSianature, bool, SetEnable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRedoButtonEnableSianature, bool, SetEnable);

UCLASS(BlueprintType, Blueprintable)
class NONOGRAM_API UBoardManager : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FBoardClearedSignature BoardClearedSignature;
	UPROPERTY(BlueprintAssignable)
	FNewBoardSetSignature NewBoardSetSignature;
	UPROPERTY(BlueprintAssignable)
	FRowLineCheckUpdatedSignature RowLineCheckUpdatedSignature;
	UPROPERTY(BlueprintAssignable)
	FColLineCheckUpdatedSignature ColLineCheckUpdatedSignature;
	UPROPERTY(BlueprintAssignable)
	FCellStateChangedSignature CellStateChangedSignature;
	UPROPERTY(BlueprintAssignable)
	FRowInfoUpdatedSignature RowInfoUpdatedSignature;
	UPROPERTY(BlueprintAssignable)
	FColInfoUpdatedSignature ColInfoUpdatedSignature;
	UPROPERTY(BlueprintAssignable)
	FUndoButtonEnableSianature UndoButtonEnableSianature;
	UPROPERTY(BlueprintAssignable)
	FRedoButtonEnableSianature RedoButtonEnableSianature;

	UBoardManager();
	virtual void BeginDestroy() override;

protected:
	FBoardInfo BoardInfo;
	FBoard Board;
	int32 RowSize;
	int32 ColSize;
	FBoardCheck BoardChecker;
	int32 InfoCount;
	int32 MatchCount;
	EBoardMode BoardMode;
	bool bAutoFill;
	bool bBoardChanged;

	void InitBoardChecker();
	void MakeBoard();
	void CheckBoard(int32 UpdatedCellX, int32 UpdatedCellY);
	void GetBlocks(const TArray<ECellState>& LineState, TArray<FBlock>& OutBlocks) const;
	bool LineMatch(const FLineInfo& LineInfo, const TArray<FBlock>& Blocks, FLineCheck& OutLineCheck) const;
	

	
	void CheckRow(int32 RowIndex, FPlayLog* PlayLog = nullptr);
	void CheckCol(int32 ColIndex, FPlayLog* PlayLog = nullptr);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ANonogramGameModeBase> GameMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UPlayLogger> Logger;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UImageMaker> ImageMaker;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FBoardInfo GetBoardInfo() const { return BoardInfo; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FBoard GetBoard() const { return Board; }
	UFUNCTION(BlueprintCallable)
	ECellState GetCellStateAt(int32 X, int32 Y) const;
	UFUNCTION(BlueprintCallable)
	FORCEINLINE EBoardMode GetBoardMode() const { return BoardMode; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetEncodedBoard() const { return UNonogramStatics::EncodeBoard(Board); }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetInfoCount() const { return BoardInfo.InfoCount; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetCurrentMatchCount() const { return MatchCount; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetRowSize() const { return RowSize; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetColSize() const { return ColSize; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetBoardChanged() const { return bBoardChanged; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetEncodedInfo() const { return UNonogramStatics::EncodeInfo(BoardInfo); }

	UFUNCTION(BlueprintCallable)
	void SetBoardInfo(FBoardInfo NewBoardInfo);
	UFUNCTION(BlueprintCallable)
	FBoardInfo SetBoardMaker(int32 NewRowSize, int32 NewColSize);
	UFUNCTION(BlueprintCallable)
	void SetAutoFill(bool bNewValue);
	UFUNCTION(BlueprintCallable)
	void SetBoardWithEncodedData(FString EncodedData);

	UFUNCTION(BlueprintCallable)
	ECellState ClickCellAt(int32 X, int32 Y, EClickMode ClickMode, bool bForce = false);
	UFUNCTION(BlueprintCallable)
	void ClickMultipleCells(TArray<FVector2D> CellLocs, EClickMode ClickMode);
	UFUNCTION(BlueprintCallable)
	bool SaveBoard(FBoardInfo& OutBoardInfo);
	UFUNCTION(BlueprintCallable)
	void ClearBoard();
	UFUNCTION(BlueprintCallable)
	void CheckAll();
	UFUNCTION(BlueprintCallable)
	void ResetBoard();

	UFUNCTION(BlueprintCallable)
	bool Undo();
	UFUNCTION(BlueprintCallable)
	bool Redo();

	UFUNCTION(BlueprintCallable)
	bool SaveInProgressImage(FString Data, FString FolderName, FString BoardName, int32 RowSize, int32 ColSize, int32 Color);
};
