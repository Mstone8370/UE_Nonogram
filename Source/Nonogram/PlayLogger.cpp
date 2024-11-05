// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayLogger.h"

UPlayLogger::UPlayLogger()
    : MaxSize(10)
    , CurrentSize(0)
    , Head(MakeShared<FLogNode>())
    , Tail(MakeShared<FLogNode>())
    , Current(Head)
{
    Head->NextNode = Tail;
    Tail->PrevNode = Head;
}

void UPlayLogger::BeginDestroy()
{
    Clear();
    Head.Reset();
    Tail.Reset();
    Current.Reset();
    
    Super::BeginDestroy();
}

void UPlayLogger::DeleteNodesAfter(const TSharedPtr<FLogNode>& Node)
{
    if (!Node.IsValid() || Node == Tail)
    {
        return;
    }

    TSharedPtr<FLogNode> It = Node->NextNode;
    while (It.IsValid() && It != Tail)
    {
        It = It->NextNode;
        CurrentSize--;
    }
    CurrentSize = FMath::Max(0, CurrentSize);
    Node->NextNode = Tail;
    Tail->PrevNode = Node;
}

bool UPlayLogger::PopFront()
{
    TSharedPtr<FLogNode> FrontNode = Head->NextNode;
    if (FrontNode.IsValid() && FrontNode != Tail)
    {
        Head->NextNode = FrontNode->NextNode;
        if (FrontNode->NextNode.IsValid())
        {
            FrontNode->NextNode->PrevNode = Head;
        }
        CurrentSize--;
        return true;
    }
    return false;
}

void UPlayLogger::AddLog(TUniquePtr<FPlayLog> NewLog)
{
    // Current 뒤의 모든 노드 제거
    DeleteNodesAfter(Current);

    // 새로운 노드 생성 후 Current 뒤에 추가
    TSharedPtr<FLogNode> NewNode = MakeShared<FLogNode>(MoveTemp(NewLog));
    NewNode->PrevNode = Current;
    NewNode->NextNode = Current->NextNode;

    if (Current->NextNode.IsValid())
    {
        Current->NextNode->PrevNode = NewNode;
    }
    Current->NextNode = NewNode;

    Current = NewNode;

    // 크기 증가
    CurrentSize++;
    while (CurrentSize > MaxSize)
    {
        // 최대 로그 크기를 넘으면 가장 오래된 로그 제거
        if (!PopFront())
        {
            break;
        }
    }
}

const FPlayLog* UPlayLogger::Undo()
{
    if (CanUndo())
    {
        const FPlayLog* Ret = Current->Log.Get();
        Current = Current->PrevNode.Pin();
        return Ret;
    }
    return nullptr;
}

const FPlayLog* UPlayLogger::Redo()
{
    if (CanRedo())
    {
        Current = Current->NextNode;
        return Current->Log.Get();
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

        TSharedPtr<FLogNode> It = Head->NextNode;
        while (It.IsValid() && It != Tail)
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
