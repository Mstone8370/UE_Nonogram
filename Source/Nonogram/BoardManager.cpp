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

    bool bConnectedWithLineStart = true; // ������ �˻��� ���� Line�� ���ۿ������� ����Ǿ��ִ���.
    bool bConnectedWithOtherBlock = true; // ������ �˻��� ���� Line�� ������ �ƴϴ��� �ٸ� ���� ����Ǿ��ִ���.
    int32 ParentBlockIdx = 0; // ������ ���� ��� ���� ����Ǿ��ִ���.

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
                // �ٸ� ���� ����Ǿ����� ������ �θ� ���� ���� ������ ����.
                ParentBlockIdx = OutBlocks.Num();
            }

            // ���� ���� ã�������� CellIdx++;
            while (CellIdx < LineState.Num() - 1)
            {
                CellIdx++;
                CS = LineState[CellIdx];
                if (CS != ECellState::ECS_Filled && CS != ECellState::ECS_NotSure)
                {
                    break;
                }
            }

            // CS: �� ���� Cell�� CellState.
            if (CS == ECellState::ECS_Blank)
            {
                // Blank�� �̹� ������ ������ ����.
                bConnectedWithOtherBlock = false;
                bConnectedWithLineStart = false;
            }
            else
            {
                // Not || Invalid
                bConnectedWithOtherBlock = true;
                // ���� ���� Cell�� ��ĭ�� �ƴϸ� ���� ����.
                // ���� Line�� Start�� ����Ǿ��ִ� ��쿡�� ���� ���·� ������.
                NewBlock.bIsClosed = bConnectedWithLineStart;
                if (CS == ECellState::ECS_Invalid)
                {
                    // ���� ���� Line�� ��.
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
        // Line�� ���� ������ ���� ����Ǿ�������, ������ ���۵� ������ ������ ������ ��������� ����.
        for (int32 i = ParentBlockIdx; i < OutBlocks.Num(); i++)
        {
            OutBlocks[i].bIsConnectedFromEnd = true;
            if (!OutBlocks[i].bIsClosed)
            {
                /*
                * ParentBlockIdx�� 0�̰�, �� ���� Line�� Start�� �ش��ϴ� ��� StartIdx�� 0�̹Ƿ�
                * StartIdx - 1�� ������� -1�� ���ͼ� �ε��� ������ �߻���.
                * ������ �ش� ���� bIsClosed�� false�� ��쿡�� �ش� �۾��� �ϹǷ� �׷��� ������ �߻����� ����.
                * 
                * �ε��� 0�� ���� StartOfLine�̷��� �ش� ���� Start�� ����Ǿ��ִ� �Ͱ� ����������.
                * �׷��� ��Ȳ�� �ε��� 0�� ���� ������ ���� ����Ǿ�������,
                * ������ ���� Line�� End�� ����Ǿ��ִ� ��Ȳ��
                * Line�� �� ĭ�� ��� ��� ���� Line�� ���� ��ο� ����� ��Ȳ�̹Ƿ�,
                * ��� ���� bIsClosed�� �̹� true�� �����Ǿ����� ���̱� ����.
                */
                OutBlocks[i].bIsClosed = (LineState[OutBlocks[i].StartIdx - 1] == ECellState::ECS_Not);
            }
        }
    }
}

bool UBoardManager::LineMatch(const FLineInfo& LineInfo, const TArray<FBlock>& Blocks, FLineCheck& OutLineCheck) const
{
    // 0. ��ó��
    const int32 LineInfoNum = LineInfo.Infos.Num();
    OutLineCheck.Fill(false, LineInfoNum);

    // Info�� 0�� ��� �����ϰ� Ȯ���ϰ� return.
    if (Blocks.Num() == 0)
    {
        if (LineInfoNum == 1 && LineInfo[0] == 0)
        {
            OutLineCheck.Fill(true, 1);
        }
        return false;
    }

    // ��� ���� Line�� ���ʿ� ����Ǿ��ִ���, �� ĭ�� �ϳ��� ������ Ȯ��.
    // ��� ����Ǿ��ִ����� �� �ϳ��� ���� �� �� ����.
    const bool bAllConnected = (Blocks[0].bIsConnectedFromStart && Blocks[0].bIsConnectedFromEnd);

    // 1. �ٸ� ��캸�� ���� �켱�Ǵ� ��� Ȯ��.
    // 1.1. ��� �������� Ȯ��.
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
    // 1.2. ��� ����Ǿ������� ���� ������ Info�� �������� ������ ��� Ʋ�� ������ ����.
    if (Blocks.Num() > LineInfoNum && bAllConnected)
    {
        OutLineCheck.Fill(false, LineInfoNum);
        return false;
    }

    // 2. �Ϻκи� ������ ���.
    // Line�� Start�� ����Ǿ��ִ� ������ ��������, ���̰� Info�� �ٸ��� �����ִ� ���� �����ϴ���.
    bool bWrongAndClosedBlockExist = false;
    if (Blocks[0].bIsConnectedFromStart)
    {
        // 2.1. Line�� Start�� ����� ���鸸 Ȯ��. (������ �˻�)
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
                // Line�� End�� �ش��ϴ� ���� ���� ������ �˻� ����� �ƴϸ�, ������ Ȯ���� ������.
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
                    // �ڿ��ִ� ������ Line�� Start�� ����Ǿ��־
                    // Info�� �ٸ� ���� �ִٸ� ���� ������ Ȯ������ ����.

                    // ���� �����ִ� ���� Line�� Start�� ����� ������ ���̸鼭
                    // ���̰� Info�� �ٸ��� �����ִ��� Ȯ��.
                    bool bLastOfStart = (i + 1 >= Blocks.Num() || !Blocks[i + 1].bIsConnectedFromStart);
                    if (bLastOfStart && CurrentBlock.bIsClosed)
                    {
                        // �� ���� �ٸ� ���� �̾����� �ϳ��� �� ���ɼ��� ����.
                        bWrongAndClosedBlockExist = true;
                    }
                    break;
                }
            }
            else
            {
                // Line�� End�� ����� ������ �Ʒ����� Ȯ����.
                break;
            }
        }
    }
    if (Blocks.Last().bIsConnectedFromEnd)
    {
        // 2.2. Line�� End�� ����� ���鸸 Ȯ��. (������ �˻�)
        for (int32 i = 0; i < LineInfoNum; i++)
        {
            const int32 BlockIdx = Blocks.Num() - 1 - i;
            if (BlockIdx < 0)
            {
                if (bAllConnected)
                {
                    /*
                    * �� �б����� ���� Line�� ��� ĭ�� ä����������, ���� ������ �����ϰ�, �����ϴ�
                    * ������ �� ��Ʈ�� �´� �����.
                    * �׷��� ���ؼ��� Info�� �ݺ��Ǵ� ������ �ְ� �� ���Ͽ� �°� ���� �����ؾ���.
                    * �׸��� ���� ������ �����ϴ� ������ ������ ������ ħ������ ������ �� �б������� ��.
                    *
                    * ���� ������ ª�� Info�� ����� ��ٸ�, �߰��� Ʋ�������� ǥ�õ� Info�� �����Ƿ�,
                    * ����ڰ� ���⿡ ���� ���� ������ ���� ���еǸ�, �� ���� �� �������� ǥ���� ���� ����.
                    *
                    * ������ ���� �� �������� ���ؼ� ��� Info�� �������� �Ǿ��ٸ�, �߸��� ����� ���޵�.
                    * (e.g. Info�� 4���ε� ������ ���̰� 2�̰� ���� ���Ͽ� ��ġ�ϰ� ������.)
                    * �� ��� ��ȣ���� �߻���.
                    * �ϴ� ĭ�� �߸� ä����� �и��ϹǷ�, ��� Info�� Ʋ�������� ������ �ڿ�
                    * �������� ��ȣ�� üũ�� ��.
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
                // Line�� Start�� �ش��ϴ� ���� ���� ������ �˻� ����� �ƴϸ�, �̹� Ȯ�εǾ���.
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
                        * ������ �̹� Ȯ���� ���� �ٽ� Ȯ���ϰ� ����.
                        * ������ �ٸ� ���� Ȯ������ �� �ش��ϴ� Info�� �¾Ƽ� true�� �Ǿ�������,
                        * �̹����� ���� Info�� ���� true��� �Ǵ��ϰ�����.
                        * 
                        * �̷� ��Ȳ�� �ΰ��� ��쿡�� �߻���.
                        * (1) ���� Info���� ���Ƽ� ������ Ȯ�ε� Info�� �ٽ� ����.
                        *     �ϳ��� Info�� �� ���� ��Ī�� �� �ִ� ����ε�, �׷��� ���ؼ��� �� ����
                        *     ũ�Ⱑ ���ƾ���.
                        *     
                        *     ������ ������ Ȯ������ �� �� ���� ������ Info�� �޶� Ʋ�ȴٸ� �̰������� 
                        *     Ʋ�����̸�, �ΰ��� ���� �� Info�� ���� ä�����ٰ� ���� ��Ʊ� ������,
                        *     (e.g. ���� ĭ�� ä����ִ� �߰������ε� ������ ���� ä�� ������ �߰���
                        *     ä����ִ� ���.)
                        *     �� Info�� Ʋ�ȴٰ� ǥ�õǴ� ���� ������ ����.
                        *     
                        *     ������ ���� ũ�Ⱑ �� �� Info�� ��ġ�Ѵٸ� �� ������ �� Info�� ����
                        *     ä�����ٰ� �� �� ������, �ϳ��� Info�� ���� �ΰ��� ���� �����ϴ°�
                        *     �� Line�� �ƿ� �߸� ä���� ��Ȳ��.
                        * 
                        * (2) ��� ������ Line�� ���ʿ� ����Ǿ��ִµ� ���� ������ Info�� �������� ����.
                        *     �׸��� Info�� �ݺ��Ǵ� ������ �ְ� �� ���Ͽ� �°� ���� �����ؾ���.
                        *     �� �б����� ���� ���ؼ��� ���� ���� �����ϴ� ������ ������ ������ ħ���ؾ���.
                        *     ��ȣ�� ��Ȳ�̹Ƿ� �ϴ� ��� Ʋ�������� ǥ���ϰ� ��ȣ�� üũ�� ��.
                        * 
                        * �� ��� ������̵� �� Line�� �߸� ä�������� ��Ȯ�ϱ⿡ ��� Ʋ�������� ������.
                        */
                        OutLineCheck.Fill(false, LineInfoNum);
                        break;
                    }
                }
                else
                {
                    if (Blocks.Num() > LineInfoNum)
                    {
                        // ���� ���� ������ Info�� �������� ���ٸ�
                        // ��� ������ �̾����� ���� ������ �پ�� ���� ����ؾ� ��.

                        // �� ���� Line�� End�� ����� ������ ������.
                        bool bLastOfEnd = (BlockIdx - 1 < 0 || !Blocks[BlockIdx - 1].bIsConnectedFromEnd);
                        if (bLastOfEnd)
                        {
                            if (CurrentBlock.bIsClosed || bWrongAndClosedBlockExist)
                            {
                                // �׷��� ��쿡 �� ���� �����ְų� ������ �׷��� ���� �����ִٸ�
                                // �ٸ� ���� �̾����� ���� ������ �پ�� ���ɼ��� ����.
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

    // 2.3. ������ �ƴ����� Info�� ��� ���� ���.
    if (OutLineCheck.MatchCount == LineInfoNum)
    {
        /*
        * ���� ���� ���°� �� Line�� ��¥ �����̾��ٸ� �Լ��� ó���κп��� Ȯ�εǾ��� ����.
        * �׸��� �� �б����� �����Ϸ��� �˻��� �߻��� �� �ִ� Ư�� ���̽��� �ƴϰ�,
        * ����� ������ Info�� ��� ������, �߰��� ������� ���� ���� �Ѱ� �̻� �����ϴ� ����.
        * �̷� ��� ��� Ʋ�������� ������. (��ȣ�� üũ ����.)
        */
        OutLineCheck.Fill(false, LineInfoNum);
        return false;
    }

    // 2.4. ��ȣ�� üũ.
    if (bAllConnected)
    {
        // ���� ���� Line�� Start�ų� End���, ��� Info�� �ش��ϴ����� ��Ȯ�ϹǷ�
        // ���������� �ѹ� �� Ȯ����.
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
            // AutoFill�� PlayLog�� ���� ������ ��.
            // Undo�� Redo�� PlayLog ���� �����ϴµ� �̶��� AutoFill�� ����Ǹ� ��Ȳ�� ����.
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
            // AutoFill�� PlayLog�� ���� ������ ��.
            // Undo�� Redo�� PlayLog ���� �����ϴµ� �̶��� AutoFill�� ����Ǹ� ��Ȳ�� ����.
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
            // Cell�� ������Ʈ�ϰų� Board�� Ȯ���� �ʿ䰡 ����.
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
            // �̹� ��ǥ ���¿� �����Ƿ� continue.
            continue;
        }
        if (TargetCellState != ECellState::ECS_Blank && CurrentCellState != ECellState::ECS_Blank)
        {
            // ĭ�� ä�����ϴ� ��Ȳ�ε�, �� ĭ�� �̹� �ٸ������� ä�����־ continue;
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
