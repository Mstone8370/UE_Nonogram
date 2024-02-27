// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FPressedSignature, FKey);

/**
 * 
 */
class NONOGRAM_API SNonogramCell : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNonogramCell)
		: _Color(FLinearColor::Transparent)
		{
		}
		SLATE_ARGUMENT(FSlateBrush*, Brush)

		SLATE_ARGUMENT(FLinearColor, Color)

		SLATE_EVENT(FPressedSignature, OnPressedArg)

		SLATE_EVENT(FSimpleDelegate, OnReleasedArg)

		SLATE_EVENT(FSimpleDelegate, OnHoveredArg)

		SLATE_EVENT(FSimpleDelegate, OnUnhoveredArg)
	SLATE_END_ARGS()

	SNonogramCell();

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	//~ SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	void SetBrush(FSlateBrush* InBrush);
	void SetColor(FLinearColor InColorAndOpacity);

private:
	/** The delegate to execute when the button is pressed */
	FPressedSignature OnPressed;
	/** The delegate to execute when the button is released */
	FSimpleDelegate OnReleased;
	/** The delegate to execute when the button is hovered */
	FSimpleDelegate OnHovered;
	/** The delegate to execute when the button exit the hovered state */
	FSimpleDelegate OnUnhovered;

	FInvalidatableBrushAttribute Brush;
	FLinearColor Color;
};
