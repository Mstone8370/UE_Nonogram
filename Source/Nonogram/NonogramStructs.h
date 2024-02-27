// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "NonogramStructs.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ECellState : uint8
{
    ECS_Blank UMETA(DisplayName = "Blank"),
    ECS_Filled UMETA(DisplayName = "Filled"),
    ECS_Not UMETA(DisplayName = "Not"),
    ECS_NotSure UMETA(DisplayName = "NotSure"),

    ECS_Invalid UMETA(DisplayName = "Invalid")
};

UENUM(BlueprintType)
enum class EClickMode : uint8
{
    ECM_Blank UMETA(DisplayName = "Blank"),
    ECM_Fill UMETA(DisplayName = "Fill"),
    ECM_Not UMETA(DisplayName = "Not"),
    ECM_NotSure UMETA(DisplayName = "NotSure"),

    ECM_None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class ESolveResult : uint8
{
    ESR_Solved UMETA(DisplayName = "Solved"),
    ESR_Ambiguous UMETA(DisplayName = "Ambiguous"),
    ESR_Failed UMETA(DisplayName = "Failed"),

    ESR_None UMETA(DisplayName = "None")
};

// Board Structs

USTRUCT(BlueprintType)
struct FCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LocationX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LocationY;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECellState CellState;

    void Reset()
    {
        CellState = ECellState::ECS_Blank;
    }
};

USTRUCT(BlueprintType)
struct FCellRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCell> Cells;

    FCell& operator[](int32 Idx)
    {
        return Cells[Idx];
    }

    FCell operator[](int32 Idx) const
    {
        return Cells[Idx];
    }

    void Clear()
    {
        Cells.Empty();
    }

    void Reset()
    {
        for (FCell& Cell : Cells)
        {
            Cell.Reset();
        }
    }

    ~FCellRow()
    {
        Clear();
    }
};

USTRUCT(BlueprintType)
struct FBoard
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCellRow> Rows;

    FCellRow& operator[](int32 Idx)
    {
        return Rows[Idx];
    }

    FCellRow operator[](int32 Idx) const
    {
        return Rows[Idx];
    }

    void Clear()
    {
        for (auto& x : Rows)
        {
            x.Clear();
        }
        Rows.Empty();
    }

    void Reset()
    {
        for (auto& x : Rows)
        {
            x.Reset();
        }
    }

    ~FBoard()
    {
        Clear();
    }
};


// Board Info Structs

USTRUCT(BlueprintType)
struct FLineInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> Infos;

    int32& operator[](int32 Idx)
    {
        return Infos[Idx];
    }

    int32 operator[](int32 Idx) const
    {
        return Infos[Idx];
    }

    void Clear()
    {
        Infos.Empty();
    }

    ~FLineInfo()
    {
        Clear();
    }
};

USTRUCT(BlueprintType)
struct FBoardInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RowNum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ColNum;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLineInfo> Rows;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLineInfo> Cols;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InfoCount = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* PuzzleImage;

    FBoardInfo()
    {
        RowNum = 0;
        ColNum = 0;
        InfoCount = 0;
    }

    int32 GetInfoCount()
    {
        if (InfoCount > 0)
        {
            return InfoCount;
        }
        else
        {
            InfoCount = 0;
            for (const FLineInfo& i : Rows)
            {
                InfoCount += i.Infos.Num();
            }
            for (const FLineInfo& i : Cols)
            {
                InfoCount += i.Infos.Num();
            }
            return InfoCount;
        }
    }

    void Clear()
    {
        DisplayText = FText();
        RowNum = -1;
        ColNum = -1;
        InfoCount = -1;
        for (auto& x : Rows)
        {
            x.Clear();
        }
        Rows.Empty();
        for (auto& x : Cols)
        {
            x.Clear();
        }
        Cols.Empty();
    }

    ~FBoardInfo()
    {
        Clear();
    }
};


// Board Checker Structs

USTRUCT(BlueprintType)
struct FLineCheck
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<bool> MatchStates;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MatchCount = 0;

    FLineCheck()
    {
        MatchCount = 0;
    }

    ~FLineCheck()
    {
        Clear();
    }

    void Fill(bool Value, int32 Size)
    {
        Clear();
        if (Size > 0)
        {
            MatchStates.Init(Value, Size);
            if (Value)
            {
                MatchCount = Size;
            }
            else
            {
                MatchCount = 0;
            }
        }
    }

    void SetState(int32 index, bool bNewState)
    {
        if (index < 0)
        {
            index = MatchStates.Num() + index;
        }
        if (0 <= index && index < MatchStates.Num())
        {
            if (bNewState != MatchStates[index])
            {
                MatchStates[index] = bNewState;
                if (bNewState)
                {
                    MatchCount++;
                }
                else
                {
                    MatchCount--;
                }
            }
        }
    }

    void Clear()
    {
        MatchCount = 0;
        MatchStates.Empty();
    }

    void Reset()
    {
        int32 Num = MatchStates.Num();
        Fill(false, Num);
    }
};

USTRUCT(BlueprintType)
struct FBoardCheck
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLineCheck> Rows;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLineCheck> Cols;

    void Clear()
    {
        for (auto& x : Rows)
        {
            x.Clear();
        }
        Rows.Empty();
        for (auto& x : Cols)
        {
            x.Clear();
        }
        Cols.Empty();
    }

    void Reset()
    {
        for (auto& x : Rows)
        {
            x.Reset();
        }
        for (auto& x : Cols)
        {
            x.Reset();
        }
    }

    ~FBoardCheck()
    {
        Clear();
    }
};

USTRUCT(BlueprintType)
struct FBlock
{
    GENERATED_BODY()

    // 블럭 범위: [StartIdx, EndIdx]
    int32 StartIdx = -1;
    int32 EndIdx = -1;
    int32 Length = -1;

    bool bIsClosed = false;
    
    bool bIsConnectedFromStart = false;
    bool bIsConnectedFromEnd = false;

    bool bStartOfLine = false;
    bool bEndOfLine = false;

    // DEPRECATED
    int32 ParentBlockIdx = -1;
    bool bIsConnected = false;

    int32 GetBlockLength() const
    {
        if (StartIdx < 0 || EndIdx < 0)
        {
            return 0;
        }
        return FMath::Abs(EndIdx - StartIdx + 1);
    }

    FString ToString() const
    {
        FString ret = "";
        if (this->bStartOfLine)
        {
            ret.Append("[");
        }
        if (this->bIsConnectedFromStart)
        {
            ret.Append("[");
        }
        if (this->bIsClosed)
        {
            ret.Append("(");
        }
        ret.AppendInt(this->Length);
        if (this->bIsClosed)
        {
            ret.Append(")");
        }
        if (this->bIsConnectedFromEnd)
        {
            ret.Append("]");
        }
        if (this->bEndOfLine)
        {
            ret.Append("]");
        }
        return ret;
    }
};

// PlayLog struct

struct FPlayLog
{
    ECellState From;
    ECellState To;
    TArray<FVector2D> Cells;
    TArray<FVector2D> AutoFilledCells;

    FPlayLog()
    {
        From = ECellState::ECS_Invalid;
        To = ECellState::ECS_Invalid;
        Cells = TArray<FVector2D>();
        AutoFilledCells = TArray<FVector2D>();
    }

    ~FPlayLog()
    {
        Clear();
    }

    void Clear()
    {
        Cells.Empty();
        AutoFilledCells.Empty();
    }
};


UCLASS()
class NONOGRAM_API UNonogramStatics : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    static ECellState ClickModeToCellState(EClickMode CM)
    {
        switch (CM)
        {
        case EClickMode::ECM_Blank:
            return ECellState::ECS_Blank;
        case EClickMode::ECM_Fill:
            return ECellState::ECS_Filled;
        case EClickMode::ECM_Not:
            return ECellState::ECS_Not;
        case EClickMode::ECM_NotSure:
            return ECellState::ECS_NotSure;
        default:
            return ECellState::ECS_Invalid;
        }
    }

    static TCHAR NumToChar(int32 Num)
    {
        TCHAR ret = 'A' + Num;
        if (ret > 'Z')
        {
            ret += ('a' - 'Z' - 1);
        }
        return ret;
    }

    static int32 CharToNum(TCHAR Char)
    {
        int32 ret = Char - 'A';
        if (Char >= 'a')
        {
            ret -= ('a' - 'Z' - 1);
        }
        return ret;
    }

    static int32 CellStateToNum(ECellState CS)
    {
        if (CS == ECellState::ECS_Blank)
        {
            return 0;
        }
        else if (CS == ECellState::ECS_Filled)
        {
            return 1;
        }
        else if (CS == ECellState::ECS_Not)
        {
            return 2;
        }
        else if (CS == ECellState::ECS_NotSure)
        {
            return 3;
        }
        return 4;
    }

    static ECellState NumToCellState(int32 Num)
    {
        if (Num == 0)
        {
            return ECellState::ECS_Blank;
        }
        else if (Num == 1)
        {
            return ECellState::ECS_Filled;
        }
        else if (Num == 2)
        {
            return ECellState::ECS_Not;
        }
        else if (Num == 3)
        {
            return ECellState::ECS_NotSure;
        }
        return ECellState::ECS_Invalid;
    }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static FString EncodeBoard(const FBoard& Board)
    {
        /*
        * 0: Blank
        * 1: Filled
        * 2: Not
        * 3: NotSure
        */
        FString EncodedData = "";
        for (int32 y = 0; y < Board.Rows.Num(); y++)
        {
            int32 LenCount = 1;
            ECellState CS = Board[y][0].CellState;
            for (int32 x = 1; x < Board[y].Cells.Num(); x++)
            {
                ECellState CurrentCS = Board[y][x].CellState;
                if (CS == CurrentCS)
                {
                    LenCount++;
                }
                else
                {
                    int32 CSNum = CellStateToNum(CS);
                    EncodedData.AppendInt(CSNum);
                    EncodedData.AppendChar(NumToChar(LenCount));
                    CS = CurrentCS;
                    LenCount = 1;
                }
            }
            int32 CSNum = CellStateToNum(CS);
            EncodedData.AppendInt(CSNum);
            EncodedData.AppendChar(NumToChar(LenCount));
            EncodedData.AppendChar('|');
        }
        EncodedData.RemoveAt(EncodedData.Len() - 1, 1);
        return EncodedData;
    }

    static void DecodeBoard(const FString& EncodedData, FBoard& OutBoard)
    {
        /*
        * 0: Blank
        * 1: Filled
        * 2: Not
        * 3: NotSure
        */
        int32 x = 0;
        int32 y = 0;
        ECellState TargetCS = ECellState::ECS_Invalid;
        for (const TCHAR& c : EncodedData)
        {
            if (c == '|')
            {
                x = 0;
                y++;
                continue;
            }

            if ('0' <= c && c < '4')
            {
                TargetCS = NumToCellState(c - '0');
            }
            else
            {
                const int32 length = UNonogramStatics::CharToNum(c);
                for (int32 i = 0; i < length; i++)
                {
                    OutBoard[y][x].CellState = TargetCS;
                    x++;
                }
            }
        }
    }

    UFUNCTION(BlueprintCallable, BlueprintPure)
    static FString EncodeInfo(const FBoardInfo& BoardInfo)
    {
        FString EncodedData = TEXT("");
        for (const FLineInfo& LI : BoardInfo.Rows)
        {
            for (const int& Num : LI.Infos)
            {
                EncodedData.AppendChar(NumToChar(Num));
            }
            EncodedData.AppendChar(TCHAR(';'));
        }
        EncodedData[EncodedData.Len() - 1] = TCHAR('|');
        for (const FLineInfo& LI : BoardInfo.Cols)
        {
            for (const int& Num : LI.Infos)
            {
                EncodedData.AppendChar(NumToChar(Num));
            }
            EncodedData.AppendChar(TCHAR(';'));
        }
        EncodedData.RemoveAt(EncodedData.Len() - 1, 1);

        return EncodedData;
    }

    UFUNCTION(BlueprintCallable)
    static void DecodeInfo(const FString& EncodedInfo, FBoardInfo& OutBoardInfo)
    {
        OutBoardInfo.Clear();
        OutBoardInfo.InfoCount = 0;

        FString FrontData;
        FString BehindData;
        EncodedInfo.Split(TEXT("|"), &FrontData, &BehindData);

        TArray<FString> RowData;
        FrontData.ParseIntoArray(RowData, TEXT(";"));
        OutBoardInfo.RowNum = RowData.Num();
        for (const FString& InfoData : RowData)
        {
            FLineInfo LI = FLineInfo();
            for (const TCHAR& C : InfoData)
            {
                LI.Infos.Add(CharToNum(C));
                OutBoardInfo.InfoCount += 1;
            }
            OutBoardInfo.Rows.Add(LI);
        }

        TArray<FString> ColData;
        BehindData.ParseIntoArray(ColData, TEXT(";"));
        OutBoardInfo.ColNum = ColData.Num();
        for (const FString& InfoData : ColData)
        {
            FLineInfo LI = FLineInfo();
            for (const TCHAR& C : InfoData)
            {
                LI.Infos.Add(CharToNum(C));
                OutBoardInfo.InfoCount += 1;
            }
            OutBoardInfo.Cols.Add(LI);
        }
    }
};
