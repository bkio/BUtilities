/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BUtilitiesBlueprintFunctionLibrary.generated.h"

UCLASS()
class BUTILITIES_API UBUtilitiesBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "BUtilities")
	static TArray<FString> SortArray(UPARAM(ref) TArray<FString>& Array);

	UFUNCTION(BlueprintCallable, Category = "BUtilities")
	static TArray<FString> SplitByNewLine(const FString& Input, bool bCullEmpty = true);

	UFUNCTION(BlueprintPure, Category = "BUtilities")
	static FString AppendWithNewLine(const FString& Input1, const FString& Input2);

	UFUNCTION(BlueprintPure, Category = "BUtilities")
	static FString TrimNewLine(const FString& Input);

	UFUNCTION(BlueprintPure, Category = "BUtilities")
	static TArray<FString> SearchForText(const FString& Input, const int ResultLimit, const TArray<FString> SearchArray);
};