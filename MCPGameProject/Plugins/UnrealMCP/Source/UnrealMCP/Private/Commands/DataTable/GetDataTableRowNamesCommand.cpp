#include "Commands/DataTable/GetDataTableRowNamesCommand.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/DataTable.h"

FGetDataTableRowNamesCommand::FGetDataTableRowNamesCommand(IDataTableService& InDataTableService)
    : DataTableService(InDataTableService)
{
}

FString FGetDataTableRowNamesCommand::Execute(const FString& Parameters)
{
    // Parse parameters
    FString DataTableName;
    FString ParseError;
    
    if (!ParseParameters(Parameters, DataTableName, ParseError))
    {
        return CreateErrorResponse(ParseError);
    }
    
    // Find the DataTable using the service layer
    UDataTable* DataTable = DataTableService.FindDataTable(DataTableName);
    if (!DataTable)
    {
        return CreateErrorResponse(FString::Printf(TEXT("DataTable not found: %s"), *DataTableName));
    }
    
    // Get row names and field names using the service
    TArray<FString> RowNames;
    TArray<FString> FieldNames;
    bool bSuccess = DataTableService.GetDataTableRowNames(DataTable, RowNames, FieldNames);
    
    if (!bSuccess)
    {
        return CreateErrorResponse(TEXT("Failed to retrieve DataTable row names and field names"));
    }
    
    // Log successful operation
    UE_LOG(LogTemp, Log, TEXT("MCP DataTable: Successfully retrieved %d row names and %d field names from DataTable '%s'"), 
           RowNames.Num(), FieldNames.Num(), *DataTableName);
    
    return CreateSuccessResponse(RowNames, FieldNames);
}

FString FGetDataTableRowNamesCommand::GetCommandName() const
{
    return TEXT("get_datatable_row_names");
}

bool FGetDataTableRowNamesCommand::ValidateParams(const FString& Parameters) const
{
    // Parse JSON parameters
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Parameters);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        return false;
    }
    
    // Basic parameter validation - datatable_name is required
    FString DataTableName;
    if (!JsonObject->TryGetStringField(TEXT("datatable_name"), DataTableName) || DataTableName.IsEmpty())
    {
        return false;
    }
    
    return true;
}

bool FGetDataTableRowNamesCommand::ParseParameters(const FString& JsonString, FString& OutDataTableName, FString& OutError) const
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OutError = TEXT("Invalid JSON parameters");
        return false;
    }
    
    // Parse required datatable_name parameter
    if (!JsonObject->TryGetStringField(TEXT("datatable_name"), OutDataTableName))
    {
        OutError = TEXT("Missing required 'datatable_name' parameter");
        return false;
    }
    
    return true;
}

FString FGetDataTableRowNamesCommand::CreateSuccessResponse(const TArray<FString>& RowNames, const TArray<FString>& FieldNames) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), true);
    ResponseObj->SetStringField(TEXT("command"), GetCommandName());
    
    // Add row names array
    TArray<TSharedPtr<FJsonValue>> RowNamesJson;
    for (const FString& RowName : RowNames)
    {
        RowNamesJson.Add(MakeShared<FJsonValueString>(RowName));
    }
    ResponseObj->SetArrayField(TEXT("row_names"), RowNamesJson);
    
    // Add field names array (GUID-based internal names)
    TArray<TSharedPtr<FJsonValue>> FieldNamesJson;
    for (const FString& FieldName : FieldNames)
    {
        FieldNamesJson.Add(MakeShared<FJsonValueString>(FieldName));
    }
    ResponseObj->SetArrayField(TEXT("field_names"), FieldNamesJson);
    
    // Add metadata
    TSharedPtr<FJsonObject> Metadata = MakeShared<FJsonObject>();
    Metadata->SetStringField(TEXT("timestamp"), FDateTime::UtcNow().ToIso8601());
    Metadata->SetStringField(TEXT("operation"), TEXT("get_row_names"));
    Metadata->SetNumberField(TEXT("row_count"), RowNames.Num());
    Metadata->SetNumberField(TEXT("field_count"), FieldNames.Num());
    ResponseObj->SetObjectField(TEXT("metadata"), Metadata);
    
    // Serialize to JSON string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
    
    return OutputString;
}

FString FGetDataTableRowNamesCommand::CreateErrorResponse(const FString& ErrorMessage) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), false);
    ResponseObj->SetStringField(TEXT("error"), ErrorMessage);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
    
    return OutputString;
}

