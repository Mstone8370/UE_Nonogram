// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/NativeWidgetHost.h"
#include "NonogramCell.generated.h"

class SNonogramCell;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClickedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPressedEvent, FKey, EffectingButton);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleasedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoverEvent);

/**
 * 
 */
UCLASS()
class NONOGRAM_API UNonogramCell : public UNativeWidgetHost
{
	GENERATED_BODY()

public: 
	virtual TSharedRef<SWidget> RebuildWidget() override;
	void ReleaseSlateResources(bool bReleaseChildren);

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnClickedEvent OnClicked;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnPressedEvent OnPressed;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnReleasedEvent OnReleased;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHoverEvent OnHovered;
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnHoverEvent OnUnhovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FSlateBrush Brush;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor Color;

	UFUNCTION(BlueprintCallable)
	void SetColor(FLinearColor InColor);
	
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

protected:
	void SlateHandlePressed(FKey EffectingButton);
	void SlateHandleReleased();
	void SlateHandleHovered();
	void SlateHandleUnhovered();

	TSharedPtr<SNonogramCell> Cell;
};
