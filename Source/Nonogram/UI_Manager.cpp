// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_Manager.h"

// Sets default values for this component's properties
UUI_Manager::UUI_Manager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUI_Manager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUI_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

