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
    FPlayLog* Log;
    FLogNode* PrevNode;
    FLogNode* NextNode;
    
    FLogNode() :
        Log(nullptr),
        PrevNode(nullptr),
        NextNode(nullptr)
    {
    }

    FLogNode(FPlayLog* NewLog) :
        FLogNode()
    {
        SetLog(NewLog);
    }

    ~FLogNode()
    {
        if (Log)
        {
            Log->Clear();
        }
        PrevNode = nullptr;
        NextNode = nullptr;
    }

    void SetLog(FPlayLog* NewLog)
    {
        delete Log;
        Log = NewLog;
    }
};

UCLASS(BlueprintType, Blueprintable)
class NONOGRAM_API UPlayLogger : public UObject
{
    GENERATED_BODY()

public:
    UPlayLogger();
    UPlayLogger(int32 Size);
    virtual void BeginDestroy() override;
    
private:
    int32 MaxSize;
    int32 CurrentSize;
    FLogNode* Head;
    FLogNode* Tail;
    FLogNode* Current;

    void DeleteNodesAfter(FLogNode* Node);

public:
    void AddLog(FPlayLog* NewLog);

    FPlayLog* Undo();
    FPlayLog* Redo();

    bool CanUndo() const;
    bool CanRedo() const;

    void Clear();

    FString ToString() const;
};
