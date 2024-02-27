// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NonogramStructs.h"

/**
 * 
 */

struct Row
{
	TArray<bool> Line;

	~Row()
	{
		Line.Empty();
	}

	bool operator[](int32 Idx) const
	{
		return Line[Idx];
	}

	bool& operator[](int32 Idx)
	{
		return Line[Idx];
	}
};

class NONOGRAM_API NonogramSolver
{
public:
	NonogramSolver();
	~NonogramSolver();

	ESolveResult Solve(const FBoardInfo& NewBoardInfo);

protected:
	TArray<Row> Board;

	void MakeBoard(int32 RowSize, int32 ColSize);
	bool FillRow(const FBoardInfo& BoardInfo, int32 RowIdx);
};
