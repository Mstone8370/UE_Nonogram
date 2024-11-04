// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NonogramStructs.h"
#include "PlayLogger.generated.h"

/**
 * 
 */

struct FLogNode
{
    TUniquePtr<FPlayLog> Log;
    TWeakPtr<FLogNode> PrevNode; // ��ȯ ���� ���� �������� Weak ������ ���
    TSharedPtr<FLogNode> NextNode;
    
    FLogNode()
        : Log(nullptr)
        , PrevNode(nullptr)
        , NextNode(nullptr)
    {}

    explicit FLogNode(TUniquePtr<FPlayLog> NewLog)
        : Log(MoveTemp(NewLog))
        , PrevNode(nullptr)
        , NextNode(nullptr)
    {}

    ~FLogNode()
    {
        if (Log.IsValid())
        {
            Log->Clear();
        }
        PrevNode.Reset();
        NextNode.Reset();
    }

    void SetLog(TUniquePtr<FPlayLog> NewLog)
    {
        Log = MoveTemp(NewLog);
    }

    // ���� �����ڿ� ���� ���� ������ ����
    FLogNode(const FLogNode&) = delete;
    FLogNode& operator=(const FLogNode&) = delete;
};

UCLASS(BlueprintType, Blueprintable)
class NONOGRAM_API UPlayLogger : public UObject
{
    GENERATED_BODY()

public:
    UPlayLogger();
    virtual void BeginDestroy() override;
    
private:
    int32 MaxSize;
    int32 CurrentSize;
    TSharedPtr<FLogNode> Head;
    TSharedPtr<FLogNode> Tail;
    TSharedPtr<FLogNode> Current;

    void DeleteNodesAfter(const TSharedPtr<FLogNode>& Node);

public:
    void AddLog(TUniquePtr<FPlayLog> NewLog);

    const FPlayLog* Undo();
    const FPlayLog* Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    void Clear();

    FString ToString() const;
};
