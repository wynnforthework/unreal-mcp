#include "Services/BlueprintActionService.h"
#include "Commands/UnrealMCPBlueprintActionCommands.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

TSharedPtr<FJsonObject> FBlueprintActionService::GetActionsForPin(
    const FString& PinType,
    const FString& PinSubcategory,
    const FString& SearchFilter,
    int32 MaxResults)
{
    if (MaxResults <= 0 || MaxResults > 1000)
    {
        TSharedPtr<FJsonObject> ErrorResponse = MakeShared<FJsonObject>();
        ErrorResponse->SetBoolField(TEXT("success"), false);
        ErrorResponse->SetStringField(TEXT("error"), TEXT("MaxResults must be between 1 and 1000"));
        return ErrorResponse;
    }
    
    FString JsonResult = UUnrealMCPBlueprintActionCommands::GetActionsForPin(
        PinType,
        PinSubcategory,
        SearchFilter,
        MaxResults
    );
    
    return ParseJsonResult(JsonResult);
}

TSharedPtr<FJsonObject> FBlueprintActionService::ParseJsonResult(const FString& JsonString)
{
    TSharedPtr<FJsonObject> ParsedResult;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (FJsonSerializer::Deserialize(Reader, ParsedResult) && ParsedResult.IsValid())
    {
        return ParsedResult;
    }
    else
    {
        TSharedPtr<FJsonObject> ErrorResponse = MakeShared<FJsonObject>();
        ErrorResponse->SetBoolField(TEXT("success"), false);
        ErrorResponse->SetStringField(TEXT("error"), TEXT("Failed to parse blueprint action result"));
        return ErrorResponse;
    }
}