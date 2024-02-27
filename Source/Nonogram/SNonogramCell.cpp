// Fill out your copyright notice in the Description page of Project Settings.


#include "SNonogramCell.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SNonogramCell::Construct(const FArguments& InArgs)
{
	Brush = FInvalidatableBrushAttribute(InArgs._Brush);
	Color = InArgs._Color;
	OnPressed = InArgs._OnPressedArg;
	OnReleased = InArgs._OnReleasedArg;
	OnHovered = InArgs._OnHoveredArg;
	OnUnhovered = InArgs._OnUnhoveredArg;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

SNonogramCell::SNonogramCell()
{}

int32 SNonogramCell::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* SlateBrush = Brush.Get();
	FLinearColor FinalColor = Color;
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), SlateBrush, ESlateDrawEffect::None, FinalColor);
	return LayerId;
}

FReply SNonogramCell::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();
	FKey EffectingButton = MouseEvent.GetEffectingButton();
	if (EffectingButton == EKeys::LeftMouseButton || EffectingButton == EKeys::RightMouseButton || MouseEvent.IsTouchEvent())
	{
		OnPressed.ExecuteIfBound(EffectingButton);
		Reply = FReply::Handled();
	}
	return Reply;
}

FReply SNonogramCell::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnReleased.ExecuteIfBound();
	return FReply::Handled();
}

void SNonogramCell::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnHovered.ExecuteIfBound();
}

void SNonogramCell::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	OnUnhovered.ExecuteIfBound();
}

void SNonogramCell::SetBrush(FSlateBrush* InBrush)
{
	Brush.SetImage(*this, InBrush);
}

void SNonogramCell::SetColor(FLinearColor InColorAndOpacity)
{
	Color = InColorAndOpacity;
}
