// Fill out your copyright notice in the Description page of Project Settings.


#include "NonogramCell.h"

#include "SNonogramCell.h"

TSharedRef<SWidget> UNonogramCell::RebuildWidget()
{
    Cell = SNew(SNonogramCell)
        .Brush(&Brush)
        .Color(Color)
        .OnPressedArg_UObject(this, &ThisClass::SlateHandlePressed)
        .OnReleasedArg(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
        .OnHoveredArg_UObject(this, &ThisClass::SlateHandleHovered)
        .OnUnhoveredArg_UObject(this, &ThisClass::SlateHandleUnhovered)
        ;
    return Cell.ToSharedRef();
}

void UNonogramCell::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);

    Cell.Reset();
}

void UNonogramCell::SetColor(FLinearColor InColor)
{
    Color = InColor;
    if (Cell.IsValid())
    {
        Cell->SetColor(Color);
    }
}

void UNonogramCell::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    Cell->SetBrush(&Brush);
    Cell->SetColor(Color);
}

void UNonogramCell::SlateHandlePressed(FKey EffectingButton)
{
    OnPressed.Broadcast(EffectingButton);
}

void UNonogramCell::SlateHandleReleased()
{
    OnReleased.Broadcast();
}

void UNonogramCell::SlateHandleHovered()
{
    OnHovered.Broadcast();
}

void UNonogramCell::SlateHandleUnhovered()
{
    OnUnhovered.Broadcast();
}
