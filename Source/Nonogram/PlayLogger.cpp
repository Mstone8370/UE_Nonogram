// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayLogger.h"

UPlayLogger::UPlayLogger() :
    MaxSize(10),
    CurrentSize(0)
{
    Head = new FLogNode();
    Tail = new FLogNode();
    Current = Head;

    Head->NextNode = Tail;
    Tail->PrevNode = Head;
}

UPlayLogger::UPlayLogger(int32 Size) :
    UPlayLogger()
{
    if (Size > 0)
    {
        MaxSize = Size;
    }
}

void UPlayLogger::BeginDestroy()
{
    Clear();
    delete Head;
    delete Tail;
    Head = nullptr;
    Tail = nullptr;
    Current = nullptr;
    
    Super::BeginDestroy();
}

void UPlayLogger::DeleteNodesAfter(FLogNode* Node)
{
    if (Node == Tail)
    {
        return;
    }

    FLogNode* It = Node->NextNode;
    while (It != Tail)
    {
        FLogNode* DelNode = It;
        It = It->NextNode;
        delete DelNode;

        CurrentSize--;
    }
    Node->NextNode = Tail;
    Tail->PrevNode = Node;
}

void UPlayLogger::AddLog(FPlayLog* NewLog)
{
    // Current 뒤의 모든 노드 제거
    DeleteNodesAfter(Current);

    // 새로운 노드 생성 후 Current 뒤에 추가
    FLogNode* NewNode = new FLogNode(NewLog);
    NewNode->PrevNode = Current;
    NewNode->NextNode = Current->NextNode;

    Current->NextNode->PrevNode = NewNode;
    Current->NextNode = NewNode;

    Current = NewNode;

    // 
    CurrentSize++;
    if (CurrentSize > MaxSize)
    {
        // 최대 로그 크기를 넘으면 가장 오래된 로그 제거
        FLogNode* OldestNode = Head->NextNode;
        Head->NextNode = OldestNode->NextNode;
        OldestNode->NextNode->PrevNode = Head;
        delete OldestNode;

        CurrentSize = MaxSize;
    }
}

FPlayLog* UPlayLogger::Undo()
{
    if (CanUndo())
    {
        FPlayLog* Ret = Current->Log;
        Current = Current->PrevNode;
        return Ret;
    }
    return nullptr;
}

FPlayLog* UPlayLogger::Redo()
{
    if (CanRedo())
    {
        Current = Current->NextNode;
        return Current->Log;
    }
    return nullptr;
}

bool UPlayLogger::CanUndo() const
{
    return Current != Head;
}

bool UPlayLogger::CanRedo() const
{
    return Current->NextNode != Tail;
}

void UPlayLogger::Clear()
{
    DeleteNodesAfter(Head);
    Current = Head;
    CurrentSize = 0;
}

FString UPlayLogger::ToString() const
{
    FString Ret = "[PlayLog] Size: ";
    Ret.AppendInt(CurrentSize);

    if (CurrentSize)
    {
        Ret.Append(" // ");

        FLogNode* It = Head->NextNode;
        while (It != Tail)
        {
            if (It == Current)
            {
                Ret.Append("**");
            }
            Ret.Append("[From: ");
            ECellState CS = It->Log->From;
            if (CS == ECellState::ECS_Blank)
            {
                Ret.Append("Blk");
            }
            else if (CS == ECellState::ECS_Filled)
            {
                Ret.Append("F");
            }
            else if (CS == ECellState::ECS_Not)
            {
                Ret.Append("N");
            }
            else if (CS == ECellState::ECS_NotSure)
            {
                Ret.Append("NS");
            }
            else
            {
                Ret.Append("Inv");
            }
            Ret.Append(", To: ");
            CS = It->Log->To;
            if (CS == ECellState::ECS_Blank)
            {
                Ret.Append("Blk");
            }
            else if (CS == ECellState::ECS_Filled)
            {
                Ret.Append("F");
            }
            else if (CS == ECellState::ECS_Not)
            {
                Ret.Append("N");
            }
            else if (CS == ECellState::ECS_NotSure)
            {
                Ret.Append("NS");
            }
            else
            {
                Ret.Append("Inv");
            }
            Ret.Append("]");
            if (It == Current)
            {
                Ret.Append("**");
            }
            Ret.Append(" -> ");
            
            It = It->NextNode;
        }
        Ret.RemoveAt(Ret.Len() - 4, 4);
    }
    return Ret;
}
