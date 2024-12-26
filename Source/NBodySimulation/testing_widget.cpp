// Fill out your copyright notice in the Description page of Project Settings.


#include "testing_widget.h"
#include "Components/TextBlock.h"
void Utesting_widget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemTitle)
	{
		ItemTitle->SetText(FText::FromString(TEXT("Hello world!")));
	}

}