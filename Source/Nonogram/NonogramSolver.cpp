// Fill out your copyright notice in the Description page of Project Settings.


#include "NonogramSolver.h"

NonogramSolver::NonogramSolver()
{
}

NonogramSolver::~NonogramSolver()
{
    
}

ESolveResult NonogramSolver::Solve(const FBoardInfo& BoardInfo)
{
    MakeBoard(BoardInfo.RowNum, BoardInfo.ColNum);

    FillRow(BoardInfo, 0);

    return ESolveResult::ESR_None;
}

void NonogramSolver::MakeBoard(int32 RowSize, int32 ColSize)
{
    Board.Empty();
    for (int32 i = 0; i < RowSize; i++)
    {
        Row r;
        r.Line.Init(false, ColSize);
        Board.Add(r);
    }
}

bool NonogramSolver::FillRow(const FBoardInfo& BoardInfo, int32 RowIdx)
{


    return false;
}
