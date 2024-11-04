// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardManager.h"

#include "NonogramGameModeBase.h"
#include "PlayLogger.h"
#include "ImageMaker.h"

UBoardManager::UBoardManager()
    :
    RowSize(-1),
    ColSize(-1),
    InfoCount(-1),
    MatchCount(-1),
    BoardMode(EBoardMode::EBM_None),
    bAutoFill(false),
    bBoardChanged(false),
    ImageMaker(nullptr)
{
    // Logger = new UPlayLogger();
}

void UBoardManager::BeginDestroy()
{
    Super::BeginDestroy();
}

void UBoardManager::InitBoardChecker()
{
    BoardChecker.Rows.Empty();
    BoardChecker.Cols.Empty();
    for (int32 i = 0; i < RowSize; i++)
    {
        FLineCheck LC;
        for (int32 j = 0; j < BoardInfo.Rows.Num(); j++)
        {
            LC.MatchStates.Add(false);
        }
        BoardChecker.Rows.Add(LC);
    }
    for (int32 i = 0; i < ColSize; i++)
    {
        FLineCheck LC;
        for (int32 j = 0; j < BoardInfo.Cols.Num(); j++)
        {
            LC.MatchStates.Add(false);
        }
        BoardChecker.Cols.Add(LC);
    }
}

void UBoardManager::MakeBoard()
{
    Board.Rows.Empty();

    for (int32 i = 0; i < RowSize; i++)
    {
        FCellRow CR;
        for (int32 j = 0; j < ColSize; j++)
        {
            FCell Cell;
            Cell.CellState = ECellState::ECS_Blank;
            Cell.LocationX = j;
            Cell.LocationY = i;
            CR.Cells.Add(Cell);
        }
        Board.Rows.Add(CR);
    }
}

void UBoardManager::CheckBoard(int32 UpdatedCellX, int32 UpdatedCellY)
{
    CheckRow(UpdatedCellY);

    CheckCol(UpdatedCellX);

    // 
    if (MatchCount == InfoCount)
    {
        // Game Clear
        UE_LOG(LogTemp, Warning, TEXT("Done!!!!!!!!!!!!!"));
        BoardClearedSignature.Broadcast();
    }
}

void UBoardManager::GetBlocks(const TArray<ECellState>& LineState, TArray<FBlock>& OutBlocks) const
{
    OutBlocks.Empty();

    bool bConnectedWithLineStart = true; // 앞으로 검사할 블럭이 Line의 시작에서부터 연결되어있는지.
    bool bConnectedWithOtherBlock = true; // 앞으로 검사할 블럭이 Line의 시작이 아니더라도 다른 블럭과 연결되어있는지.
    int32 ParentBlockIdx = 0; // 마지막 블럭이 어느 블럭에 연결되어있는지.

    int32 CellIdx = 0;
    while (CellIdx < LineState.Num())
    {
        ECellState CS = LineState[CellIdx];
        if (CS == ECellState::ECS_Invalid)
        {
            break;
        }
        else if (CS == ECellState::ECS_Blank)
        {
            bConnectedWithLineStart = false;
            bConnectedWithOtherBlock = false;
        }
        else if (CS == ECellState::ECS_Filled || CS == ECellState::ECS_NotSure)
        {
            FBlock NewBlock = FBlock();
            NewBlock.Length = 1;
            NewBlock.StartIdx = CellIdx;
            NewBlock.bStartOfLine = (CellIdx == 0);
            NewBlock.bIsConnectedFromStart = bConnectedWithLineStart;
            NewBlock.bIsClosed = false;
            if (!bConnectedWithOtherBlock)
            {
                // 다른 블럭과 연결되어있지 않으면 부모 블럭을 현재 블럭으로 지정.
                ParentBlockIdx = OutBlocks.Num();
            }

            // 블럭의 끝을 찾을때까지 CellIdx++;
            while (CellIdx < LineState.Num() - 1)
            {
                CellIdx++;
                CS = LineState[CellIdx];
                if (CS != ECellState::ECS_Filled && CS != ECellState::ECS_NotSure)
                {
                    break;
                }
            }

            // CS: 블럭 다음 Cell의 CellState.
            if (CS == ECellState::ECS_Blank)
            {
                // Blank면 이미 연결이 끊어진 상태.
                bConnectedWithOtherBlock = false;
                bConnectedWithLineStart = false;
            }
            else
            {
                // Not || Invalid
                bConnectedWithOtherBlock = true;
                // 블럭의 다음 Cell이 빈칸이 아니면 닫힌 상태.
                // 블럭이 Line의 Start와 연결되어있는 경우에만 닫힌 상태로 지정함.
                NewBlock.bIsClosed = bConnectedWithLineStart;
                if (CS == ECellState::ECS_Invalid)
                {
                    // 현재 블럭이 Line의 끝.
                    NewBlock.bEndOfLine = true;
                }
            }

            NewBlock.EndIdx = CellIdx - 1;
            NewBlock.Length = CellIdx - NewBlock.StartIdx;
            OutBlocks.Add(NewBlock);
        }
        CellIdx++;
    }

    if (OutBlocks.Num() && bConnectedWithOtherBlock)
    {
        // Line의 끝이 마지막 블럭과 연결되어있으면, 연결이 시작된 블럭부터 마지막 블럭까지 연결됨으로 설정.
        for (int32 i = ParentBlockIdx; i < OutBlocks.Num(); i++)
        {
            OutBlocks[i].bIsConnectedFromEnd = true;
            if (!OutBlocks[i].bIsClosed)
            {
                /*
                * ParentBlockIdx가 0이고, 그 블럭이 Line의 Start에 해당하는 경우 StartIdx는 0이므로
                * StartIdx - 1의 결과값은 -1이 나와서 인덱스 오류가 발생함.
                * 하지만 해당 블럭이 bIsClosed가 false인 경우에만 해당 작업을 하므로 그러한 문제는 발생하지 않음.
                * 
                * 인덱스 0의 블럭이 StartOfLine이려면 해당 블럭은 Start와 연결되어있는 것과 마찬가지임.
                * 그러한 상황에 인덱스 0의 블럭이 마지막 블럭과 연결되어있으며,
                * 마지막 블럭이 Line의 End와 연결되어있는 상황은
                * Line에 빈 칸이 없어서 모든 블럭이 Line의 양쪽 모두에 연결된 상황이므로,
                * 모든 블럭의 bIsClosed는 이미 true로 지정되어있을 것이기 때문.
                */
                OutBlocks[i].bIsClosed = (LineState[OutBlocks[i].StartIdx - 1] == ECellState::ECS_Not);
            }
        }
    }
}

bool UBoardManager::LineMatch(const FLineInfo& LineInfo, const TArray<FBlock>& Blocks, FLineCheck& OutLineCheck) const
{
    // 0. 전처리
    const int32 LineInfoNum = LineInfo.Infos.Num();
    OutLineCheck.Fill(false, LineInfoNum);

    // Info가 0인 경우 간단하게 확인하고 return.
    if (Blocks.Num() == 0)
    {
        if (LineInfoNum == 1 && LineInfo[0] == 0)
        {
            OutLineCheck.Fill(true, 1);
        }
        return false;
    }

    // 모든 블럭이 Line의 양쪽에 연결되어있는지, 빈 칸이 하나도 없는지 확인.
    // 모두 연결되어있는지는 블럭 하나만 보면 알 수 있음.
    const bool bAllConnected = (Blocks[0].bIsConnectedFromStart && Blocks[0].bIsConnectedFromEnd);

    // 1. 다른 경우보다 제일 우선되는 경우 확인.
    // 1.1. 모두 정답인지 확인.
    if (Blocks.Num() == LineInfoNum)
    {
        bool bAllMatch = true;
        for (int32 i = 0; i < LineInfoNum; i++)
        {
            if (Blocks[i].Length != LineInfo[i])
            {
                bAllMatch = false;
                break;
            }
        }
        if (bAllMatch)
        {
            OutLineCheck.Fill(true, LineInfoNum);
            return true;
        }
    }
    // 1.2. 모두 연결되어있지만 블럭의 개수가 Info의 개수보다 많으면 모두 틀린 것으로 지정.
    if (Blocks.Num() > LineInfoNum && bAllConnected)
    {
        OutLineCheck.Fill(false, LineInfoNum);
        return false;
    }

    // 2. 일부분만 정답인 경우.
    // Line의 Start에 연결되어있는 마지막 블럭이지만, 길이가 Info와 다르고 닫혀있는 블럭이 존재하는지.
    bool bWrongAndClosedBlockExist = false;
    if (Blocks[0].bIsConnectedFromStart)
    {
        // 2.1. Line의 Start와 연결된 블럭들만 확인. (정방향 검사)
        for (int32 i = 0; i < LineInfoNum; i++)
        {
            if (i >= Blocks.Num())
            {
                break;
            }

            const FBlock& CurrentBlock = Blocks[i];
            const int32 TargetLength = LineInfo[i];
            if (CurrentBlock.bEndOfLine)
            {
                // Line의 End에 해당하는 블럭은 현재 방향의 검사 대상이 아니며, 다음에 확인할 예정임.
                break;
            }

            if (CurrentBlock.bIsConnectedFromStart)
            {
                if (CurrentBlock.Length == TargetLength)
                {
                    OutLineCheck.SetState(i, true);
                }
                else
                {
                    // 뒤에있는 블럭들이 Line의 Start에 연결되어있어도
                    // Info와 다른 블럭이 있다면 다음 블럭들은 확인하지 않음.

                    // 현재 보고있는 블럭이 Line의 Start와 연결된 마지막 블럭이면서
                    // 길이가 Info와 다르고 닫혀있는지 확인.
                    bool bLastOfStart = (i + 1 >= Blocks.Num() || !Blocks[i + 1].bIsConnectedFromStart);
                    if (bLastOfStart && CurrentBlock.bIsClosed)
                    {
                        // 이 블럭은 다른 블럭과 이어져서 하나가 될 가능성이 없음.
                        bWrongAndClosedBlockExist = true;
                    }
                    break;
                }
            }
            else
            {
                // Line의 End에 연결된 블럭들은 아래에서 확인함.
                break;
            }
        }
    }
    if (Blocks.Last().bIsConnectedFromEnd)
    {
        // 2.2. Line의 End와 연결된 블럭들만 확인. (역방향 검사)
        for (int32 i = 0; i < LineInfoNum; i++)
        {
            const int32 BlockIdx = Blocks.Num() - 1 - i;
            if (BlockIdx < 0)
            {
                if (bAllConnected)
                {
                    /*
                    * 이 분기점은 현재 Line의 모든 칸이 채워져있지만, 블럭의 개수가 부족하고, 존재하는
                    * 블럭들은 다 힌트에 맞는 경우임.
                    * 그러기 위해서는 Info에 반복되는 패턴이 있고 그 패턴에 맞게 블럭이 존재해야함.
                    * 그리고 양쪽 끝에서 시작하는 패턴이 서로의 영역을 침범하지 않으면 이 분기점으로 옴.
                    *
                    * 만약 패턴이 짧고 Info가 충분히 길다면, 중간에 틀린것으로 표시될 Info가 있으므로,
                    * 사용자가 보기에 양쪽 끝의 패턴은 서로 구분되며, 두 패턴 다 정답으로 표시할 수도 있음.
                    *
                    * 하지만 양쪽 두 패턴으로 인해서 모든 Info가 정답으로 되었다면, 잘못된 결과가 전달됨.
                    * (e.g. Info는 4개인데 패턴의 길이가 2이고 블럭이 패턴에 일치하게 존재함.)
                    * 이 경우 모호성이 발생함.
                    * 일단 칸을 잘못 채운것은 분명하므로, 모든 Info를 틀린것으로 지정한 뒤에
                    * 마지막에 모호성 체크를 함.
                    */
                    if (OutLineCheck.MatchCount >= LineInfoNum)
                    {
                        OutLineCheck.Fill(false, LineInfoNum);
                    }
                }
                break;
            }

            const FBlock CurrentBlock = Blocks[BlockIdx];
            const int32 LineInfoIdx = LineInfoNum - 1 - i;
            const int32 TargetLength = LineInfo[LineInfoIdx];
            if (CurrentBlock.bStartOfLine)
            {
                // Line의 Start에 해당하는 블럭은 현재 방향의 검사 대상이 아니며, 이미 확인되었음.
                break;
            }

            if (CurrentBlock.bIsConnectedFromEnd)
            {
                if (CurrentBlock.Length == TargetLength)
                {
                    if (!OutLineCheck.MatchStates[LineInfoIdx])
                    {
                        OutLineCheck.SetState(LineInfoIdx, true);
                    }
                    else
                    {
                        /*
                        * 위에서 이미 확인한 블럭을 다시 확인하고 있음.
                        * 위에서 다른 블럭을 확인했을 때 해당하는 Info와 맞아서 true로 되어있지만,
                        * 이번에도 같은 Info를 보고 true라고 판단하고있음.
                        * 
                        * 이런 상황은 두가지 경우에서 발생함.
                        * (1) 블럭이 Info보다 많아서 이전에 확인된 Info가 다시 사용됨.
                        *     하나의 Info가 두 블럭에 매칭될 수 있는 경우인데, 그러기 위해서는 두 블럭의
                        *     크기가 같아야함.
                        *     
                        *     하지만 위에서 확인했을 때 이 블럭의 개수가 Info와 달라서 틀렸다면 이곳에서도 
                        *     틀릴것이며, 두개의 블럭이 이 Info를 위해 채워졌다고 보기 어렵기 때문에,
                        *     (e.g. 아직 칸을 채우고있는 중간과정인데 양쪽을 먼저 채운 다음에 중간을
                        *     채우고있는 경우.)
                        *     이 Info만 틀렸다고 표시되는 것은 문제가 없음.
                        *     
                        *     하지만 블럭의 크기가 둘 다 Info와 일치한다면 두 블럭들은 이 Info를 위해
                        *     채워졌다고 볼 수 있으며, 하나의 Info를 위해 두개의 블럭이 존재하는건
                        *     이 Line이 아예 잘못 채워진 상황임.
                        * 
                        * (2) 모든 블럭들이 Line의 양쪽에 연결되어있는데 블럭의 개수가 Info의 개수보다 적음.
                        *     그리고 Info에 반복되는 패턴이 있고 그 패턴에 맞게 블럭이 존재해야함.
                        *     이 분기점에 오기 위해서는 양쪽 끝에 존재하는 패턴이 서로의 영역을 침범해야함.
                        *     모호한 상황이므로 일단 모두 틀린것으로 표시하고 모호성 체크를 함.
                        * 
                        * 두 경우 어느것이든 이 Line은 잘못 채워진것은 명확하기에 모두 틀린것으로 지정함.
                        */
                        OutLineCheck.Fill(false, LineInfoNum);
                        break;
                    }
                }
                else
                {
                    if (Blocks.Num() > LineInfoNum)
                    {
                        // 만약 블럭의 개수가 Info의 개수보다 많다면
                        // 몇몇 블럭들이 이어져서 블럭의 개수가 줄어들 것을 기대해야 함.

                        // 이 블럭이 Line의 End와 연결된 마지막 블럭인지.
                        bool bLastOfEnd = (BlockIdx - 1 < 0 || !Blocks[BlockIdx - 1].bIsConnectedFromEnd);
                        if (bLastOfEnd)
                        {
                            if (CurrentBlock.bIsClosed || bWrongAndClosedBlockExist)
                            {
                                // 그러한 경우에 이 블럭이 닫혀있거나 이전의 그러한 블럭이 닫혀있다면
                                // 다른 블럭과 이어져서 블럭의 개수가 줄어들 가능성이 없음.
                                OutLineCheck.Fill(false, LineInfoNum);
                            }
                        }
                    }
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    // 2.3. 정답은 아니지만 Info가 모두 맞은 경우.
    if (OutLineCheck.MatchCount == LineInfoNum)
    {
        /*
        * 만약 블럭의 상태가 이 Line의 진짜 정답이었다면 함수의 처음부분에서 확인되었을 것임.
        * 그리고 이 분기점에 도달하려면 검사중 발생할 수 있는 특수 케이스는 아니고,
        * 연결된 블럭들이 Info와 모두 맞지만, 중간에 연결되지 않은 블럭이 한개 이상 존재하는 상태.
        * 이런 경우 모두 틀린것으로 지정함. (모호성 체크 안함.)
        */
        OutLineCheck.Fill(false, LineInfoNum);
        return false;
    }

    // 2.4. 모호성 체크.
    if (bAllConnected)
    {
        // 만약 블럭이 Line의 Start거나 End라면, 어느 Info에 해당하는지는 명확하므로
        // 마지막으로 한번 더 확인함.
        if (Blocks[0].bStartOfLine && Blocks[0].Length == LineInfo[0])
        {
            if (!OutLineCheck.MatchStates[0])
            {
                OutLineCheck.SetState(0, true);
            }
        }
        if (Blocks.Last().bEndOfLine && Blocks.Last().Length == LineInfo.Infos.Last())
        {
            if (!OutLineCheck.MatchStates.Last())
            {
                OutLineCheck.SetState(-1, true);
            }
        }
    }
    
    return false;
}

void UBoardManager::CheckRow(int32 RowIndex, FPlayLog* PlayLog)
{
    TArray<ECellState> RowCellStates;
    RowCellStates.Init(ECellState::ECS_Invalid, ColSize + 1);
    for (int32 i = 0; i <= ColSize; i++)
    {
        RowCellStates[i] = GetCellStateAt(i, RowIndex);
    }

    TArray<FBlock> RowBlocks;
    GetBlocks(RowCellStates, RowBlocks);

    if (BoardMode == EBoardMode::EBM_Play)
    {
        FLineCheck RowCheck;
        bool bAllMatch = LineMatch(BoardInfo.Rows[RowIndex], RowBlocks, RowCheck);

        RowLineCheckUpdatedSignature.Broadcast(RowIndex, RowCheck);
        MatchCount -= BoardChecker.Rows[RowIndex].MatchCount;
        MatchCount += RowCheck.MatchCount;
        BoardChecker.Rows[RowIndex] = RowCheck;

        if (bAutoFill && bAllMatch && PlayLog)
        {
            // AutoFill은 PlayLog가 있을 때에만 함.
            // Undo와 Redo는 PlayLog 없이 진행하는데 이때에 AutoFill이 적용되면 상황이 꼬임.
            for (int32 i = 0; i < ColSize; i++)
            {
                if (GetCellStateAt(i, RowIndex) == ECellState::ECS_Blank)
                {
                    Board[RowIndex][i].CellState = ECellState::ECS_Not;
                    CellStateChangedSignature.Broadcast(i, RowIndex, ECellState::ECS_Not);
                    PlayLog->AutoFilledCells.Add(FVector2D(i, RowIndex));
                }
            }
        }
    }
    else
    {
        FLineInfo& BlockInfos = BoardInfo.Rows[RowIndex];
        BoardInfo.InfoCount -= BlockInfos.Infos.Num();
        BlockInfos.Clear();
        if (RowBlocks.IsEmpty())
        {
            BlockInfos.Infos.Add(0);
        }
        else
        {
            for (const FBlock& Blk : RowBlocks)
            {
                BlockInfos.Infos.Add(Blk.Length);
            }
        }
        RowInfoUpdatedSignature.Broadcast(RowIndex, BlockInfos);
        BoardInfo.InfoCount += BlockInfos.Infos.Num();
    }
}

void UBoardManager::CheckCol(int32 ColIndex, FPlayLog* PlayLog)
{
    TArray<ECellState> ColCellStates;
    ColCellStates.Init(ECellState::ECS_Invalid, RowSize + 1);
    for (int32 i = 0; i <= RowSize; i++)
    {
        ColCellStates[i] = GetCellStateAt(ColIndex, i);
    }

    TArray<FBlock> ColBlocks;
    GetBlocks(ColCellStates, ColBlocks);

    if (BoardMode == EBoardMode::EBM_Play)
    {
        FLineCheck ColCheck;
        bool bAllMatch = LineMatch(BoardInfo.Cols[ColIndex], ColBlocks, ColCheck);

        ColLineCheckUpdatedSignature.Broadcast(ColIndex, ColCheck);
        MatchCount -= BoardChecker.Cols[ColIndex].MatchCount;
        MatchCount += ColCheck.MatchCount;
        BoardChecker.Cols[ColIndex] = ColCheck;

        if (bAutoFill && bAllMatch && PlayLog)
        {
            // AutoFill은 PlayLog가 있을 때에만 함.
            // Undo와 Redo는 PlayLog 없이 진행하는데 이때에 AutoFill이 적용되면 상황이 꼬임.
            for (int32 i = 0; i < RowSize; i++)
            {
                if (GetCellStateAt(ColIndex, i) == ECellState::ECS_Blank)
                {
                    Board[i][ColIndex].CellState = ECellState::ECS_Not;
                    CellStateChangedSignature.Broadcast(ColIndex, i, ECellState::ECS_Not);
                    PlayLog->AutoFilledCells.Add(FVector2D(ColIndex, i));
                }
            }
        }
    }
    else
    {
        FLineInfo& BlockInfos = BoardInfo.Cols[ColIndex];
        BoardInfo.InfoCount -= BlockInfos.Infos.Num();
        BlockInfos.Clear();
        if (ColBlocks.IsEmpty())
        {
            BlockInfos.Infos.Add(0);
        }
        else
        {
            for (const FBlock& Blk : ColBlocks)
            {
                BlockInfos.Infos.Add(Blk.Length);
            }
        }
        ColInfoUpdatedSignature.Broadcast(ColIndex, BlockInfos);
        BoardInfo.InfoCount += BlockInfos.Infos.Num();
    }
}

ECellState UBoardManager::GetCellStateAt(int32 X, int32 Y) const
{
    if (0 <= X && X < ColSize && 0 <= Y && Y < RowSize)
    {
        return Board.Rows[Y].Cells[X].CellState;
    }
    return ECellState::ECS_Invalid;
}

void UBoardManager::SetBoardInfo(FBoardInfo NewBoardInfo)
{
    ClearBoard();

    BoardInfo = NewBoardInfo;
    RowSize = BoardInfo.RowNum;
    ColSize = BoardInfo.ColNum;
    InfoCount = BoardInfo.GetInfoCount();
    MatchCount = 0;
    BoardMode = EBoardMode::EBM_Play;
    bBoardChanged = false;
    
    InitBoardChecker();
    MakeBoard();

    NewBoardSetSignature.Broadcast();
}

FBoardInfo UBoardManager::SetBoardMaker(int32 NewRowSize, int32 NewColSize)
{
    ClearBoard();

    BoardInfo.RowNum = NewRowSize;
    BoardInfo.ColNum = NewColSize;
    BoardInfo.InfoCount = 0;
    for (int32 i = 0; i < NewRowSize; i++)
    {
        FLineInfo LI;
        LI.Infos.Add(0);
        BoardInfo.InfoCount++;
        BoardInfo.Rows.Add(LI);
    }
    for (int32 i = 0; i < NewColSize; i++)
    {
        FLineInfo LI;
        LI.Infos.Add(0);
        BoardInfo.InfoCount++;
        BoardInfo.Cols.Add(LI);
    }

    RowSize = NewRowSize;
    ColSize = NewColSize;
    BoardMode = EBoardMode::EBM_Make;

    MakeBoard();

    NewBoardSetSignature.Broadcast();
    
    return BoardInfo;
}

/* Deprecated */
ECellState UBoardManager::ClickCellAt(int32 X, int32 Y, EClickMode ClickMode, bool bForce)
{
    if (!IN_RANGE(X, 0, ColSize) || !IN_RANGE(Y, 0, RowSize))
    {
        UE_LOG(LogTemp, Error, TEXT("[BoardManager] Cell Location X: %d, Y: %d is not valid."), X, Y);
        return ECellState::ECS_Invalid;
    }

    // ECellState TargetCellState = ECellState::ECS_Filled;
    ECellState TargetCellState = UNonogramStatics::ClickModeToCellState(ClickMode);
    if (TargetCellState == GetCellStateAt(X, Y))
    {
        if (bForce)
        {
            // Cell을 업데이트하거나 Board를 확인할 필요가 없음.
            return TargetCellState;
        }
        else
        {
            TargetCellState = ECellState::ECS_Blank;
        }
    }
    Board[Y][X].CellState = TargetCellState;
    CellStateChangedSignature.Broadcast(X, Y, TargetCellState);

    if (BoardMode == EBoardMode::EBM_Play)
    {
        CheckBoard(X, Y);
    }
    return TargetCellState;
}

void UBoardManager::ClickMultipleCells(TArray<FVector2D> CellLocs, EClickMode ClickMode)
{
    bBoardChanged = true;

    TArray<FVector2D> StateChangedCells;
    ECellState TargetCellState = UNonogramStatics::ClickModeToCellState(ClickMode);
    ECellState PrevCellState = ECellState::ECS_Invalid;

    for (const FVector2D& CellLoc : CellLocs)
    {
        int LocX = int(CellLoc.X);
        int LocY = int(CellLoc.Y);
        if (!IN_RANGE(LocX, 0, ColSize) || !IN_RANGE(LocY, 0, RowSize))
        {
            UE_LOG(LogTemp, Error, TEXT("[BoardManager] Cell Location X: %d, Y: %d is not valid."), LocX, LocY);
            continue;
        }

        ECellState CurrentCellState = GetCellStateAt(LocX, LocY);
        if (TargetCellState == CurrentCellState)
        {
            // 이미 목표 상태와 같으므로 continue.
            continue;
        }
        if (TargetCellState != ECellState::ECS_Blank && CurrentCellState != ECellState::ECS_Blank)
        {
            // 칸을 채워야하는 상황인데, 이 칸이 이미 다른것으로 채워져있어도 continue;
            continue;
        }
        if (PrevCellState == ECellState::ECS_Invalid)
        {
            PrevCellState = CurrentCellState;
        }
        if (CurrentCellState != PrevCellState)
        {
            continue;
        }

        Board[LocY][LocX].CellState = TargetCellState;
        CellStateChangedSignature.Broadcast(LocX, LocY, TargetCellState);

        StateChangedCells.Add(FVector2D(LocX, LocY));
    }

    TUniquePtr<FPlayLog> PL = MakeUnique<FPlayLog>();
    PL->From = PrevCellState;
    PL->To = TargetCellState;
    TSet<int32> Xs;
    TSet<int32> Ys;
    for (const FVector2D& CellLoc : StateChangedCells)
    {
        int LocX = int(CellLoc.X);
        int LocY = int(CellLoc.Y);
        Xs.Add(LocX);
        Ys.Add(LocY);
        PL->Cells.Add(CellLoc);
    }
    if (Xs.Num() == 1)
    {
        CheckCol(*(Xs.begin()), PL.Get());
        for (const auto& Y : Ys)
        {
            CheckRow(Y, PL.Get());
        }
    }
    else if (Ys.Num() == 1)
    {
        CheckRow(*(Ys.begin()), PL.Get());
        for (const auto& X : Xs)
        {
            CheckCol(X, PL.Get());
        }
    }

    if (Logger)
    {
        Logger->AddLog(MoveTemp(PL));
        UndoButtonEnableSianature.Broadcast(Logger->CanUndo());
        RedoButtonEnableSianature.Broadcast(Logger->CanRedo());
    }

    if (BoardMode == EBoardMode::EBM_Play)
    {
        if (MatchCount == InfoCount)
        {
            UE_LOG(LogTemp, Warning, TEXT("Done!!!!!!!!!!!!!"));
            BoardClearedSignature.Broadcast();
        }
    }
}

/* Deprecated */
bool UBoardManager::SaveBoard(FBoardInfo& OutBoardInfo)
{
    OutBoardInfo = BoardInfo;
    return true;
}

void UBoardManager::ClearBoard()
{
    Board.Clear();
    BoardInfo.Clear();
    BoardChecker.Clear();
    RowSize = 0;
    ColSize = 0;
    InfoCount = 0;
    MatchCount = 0;
    BoardMode = EBoardMode::EBM_None;
    if (Logger)
    {
        Logger->Clear();
    }
}

void UBoardManager::CheckAll()
{
    for (int32 i = 0; i < RowSize; i++)
    {
        CheckRow(i);
    }
    for (int32 i = 0; i < ColSize; i++)
    {
        CheckCol(i);
    }
}

void UBoardManager::ResetBoard()
{
    Board.Reset();
    BoardChecker.Reset();
    MatchCount = 0;
    if (Logger)
    {
        Logger->Clear();
    }
}

bool UBoardManager::Undo()
{
    if (Logger)
    {
        if (Logger->CanUndo())
        {
            const FPlayLog* Log = Logger->Undo();
            if (Log && Log->From != ECellState::ECS_Invalid && Log->To != ECellState::ECS_Invalid)
            {
                for (const FVector2D& CellLoc : Log->Cells)
                {
                    const int LocX = int(CellLoc.X);
                    const int LocY = int(CellLoc.Y);
                    Board[LocY][LocX].CellState = Log->From;
                    CellStateChangedSignature.Broadcast(LocX, LocY, Log->From);
                    CheckRow(LocY);
                    CheckCol(LocX);
                }
                for (const FVector2D& CellLoc : Log->AutoFilledCells)
                {
                    const int LocX = int(CellLoc.X);
                    const int LocY = int(CellLoc.Y);
                    if (GetCellStateAt(LocX, LocY) != ECellState::ECS_Blank)
                    {
                        Board[LocY][LocX].CellState = ECellState::ECS_Blank;
                        CellStateChangedSignature.Broadcast(LocX, LocY, ECellState::ECS_Blank);
                    }
                }
                UndoButtonEnableSianature.Broadcast(Logger->CanUndo());
                RedoButtonEnableSianature.Broadcast(Logger->CanRedo());

                return true;
            }
        }
    }
    return false;
}

bool UBoardManager::Redo()
{
    if (Logger)
    {
        if (Logger->CanRedo())
        {
            const FPlayLog* Log = Logger->Redo();
            if (Log && Log->From != ECellState::ECS_Invalid && Log->To != ECellState::ECS_Invalid)
            {
                for (const FVector2D& CellLoc : Log->Cells)
                {
                    const int LocX = int(CellLoc.X);
                    const int LocY = int(CellLoc.Y);
                    Board[LocY][LocX].CellState = Log->To;
                    CellStateChangedSignature.Broadcast(LocX, LocY, Log->To);
                    CheckRow(LocY);
                    CheckCol(LocX);
                }
                for (const FVector2D& CellLoc : Log->AutoFilledCells)
                {
                    const int LocX = int(CellLoc.X);
                    const int LocY = int(CellLoc.Y);
                    if (GetCellStateAt(LocX, LocY) != ECellState::ECS_Not)
                    {
                        Board[LocY][LocX].CellState = ECellState::ECS_Not;
                        CellStateChangedSignature.Broadcast(LocX, LocY, ECellState::ECS_Not);
                    }
                }
                UndoButtonEnableSianature.Broadcast(Logger->CanUndo());
                RedoButtonEnableSianature.Broadcast(Logger->CanRedo());

                return true;
            }
        }
    }
    return false;
}

void UBoardManager::SetAutoFill(bool bNewValue)
{
    bAutoFill = bNewValue;
}

void UBoardManager::SetBoardWithEncodedData(FString EncodedData)
{
    UNonogramStatics::DecodeBoard(EncodedData, Board);

    for (int32 y = 0; y < RowSize; y++)
    {
        for (int32 x = 0; x < ColSize; x++)
        {
            CellStateChangedSignature.Broadcast(x, y, GetCellStateAt(x, y));
        }
    }

    CheckAll();
}

bool UBoardManager::SaveInProgressImage(FString Data, FString FolderName, FString BoardName, int32 BoardRowSize, int32 BoardColSize, int32 Color)
{
    if (!ImageMaker)
    {
        return false;
    }
    
    return ImageMaker->SaveInProgressImage(Data, FolderName, BoardName, BoardRowSize, BoardColSize, Color);
}
