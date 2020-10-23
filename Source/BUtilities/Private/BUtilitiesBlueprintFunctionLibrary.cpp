/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#include "BUtilitiesBlueprintFunctionLibrary.h"

TArray<FString> UBUtilitiesBlueprintFunctionLibrary::SortArray(TArray<FString>& Array)
{
	TArray<FString> CopyArray = Array;
	CopyArray.Sort();
	return CopyArray;
}

TArray<FString> UBUtilitiesBlueprintFunctionLibrary::SplitByNewLine(const FString& Input, bool bCullEmpty)
{
	TArray<FString> Result;
	Input.ParseIntoArray(Result, TEXT("\r\n"), bCullEmpty);
	return Result;
}

FString UBUtilitiesBlueprintFunctionLibrary::AppendWithNewLine(const FString& Input1, const FString& Input2)
{
	return Input1 + TEXT("\r\n") + Input2;
}

FString UBUtilitiesBlueprintFunctionLibrary::TrimNewLine(const FString& Input)
{
	TArray<FString> Splitted = SplitByNewLine(Input, true);
	FString Trimmed;
	for (FString& Current : Splitted)
	{
		Trimmed += Current;
	}
	return Trimmed;
}

TArray<FString> UBUtilitiesBlueprintFunctionLibrary::SearchForText(const FString& Input, const int ResultLimit, const TArray<FString> SearchArray) {
	TArray<FString> ResultArray;

	for (int i = 0; i < SearchArray.Num(); ++i)
	{
		if (ResultArray.Num() >= ResultLimit)
		{
			return ResultArray;
		}

		if (SearchArray[i].Contains(Input, ESearchCase::IgnoreCase, ESearchDir::FromStart))
		{
			ResultArray.Add(SearchArray[i]);
		}
	}

	return ResultArray;
}