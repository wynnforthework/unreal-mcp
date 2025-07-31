#include "Commands/DataTable/DeleteDataTableRowCommand.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/DataTable.h"

FDeleteDataTableRowCommand::FDeleteDataTableRowCommand(IDataTableService& InDataTableService)
    : DataTableService(InDataTableService)
{
}

FString FDeleteDataTableRowCommand::Execute(const FString& Parameters)
{
    FString DataTableName;
    FString RowName;
    FString ParseError;
    
    if (!ParseParameters(Parameters, DataTableName, RowName, ParseError))
    {
        return CreateErrorResponse(ParseError);
    }
    
    // Find the DataTable
    UDataTable* DataTable = DataTableService.FindDataTable(DataTableName);
    if (!DataTable)
    {
        return CreateErrorResponse(FString::Printf(TEXT("DataTable not found: %s"), *DataTableName));
    }
    
    // Delete single row using the service (convert to array for existing service method)
    TArray<FString> RowNames = { RowName };
    TArray<FString> DeletedRows;
    TArray<FString> FailedRows;
    bool bSuccess = DataTableService.DeleteRowsFromDataTable(DataTable, RowNames, DeletedRows, FailedRows);
    
    if (!bSuccess || DeletedRows.Num() == 0)
    {
        FString ErrorMsg = FString::Printf(TEXT("Failed to delete row '%s'"), *RowName);
        if (FailedRows.Num() > 0)
        {
            ErrorMsg += TEXT(" - Row may not exist");
        }
        return CreateErrorResponse(ErrorMsg);
    }
    
    return CreateSuccessResponse(DeletedRows[0]);
}

FString FDeleteDataTableRowCommand::GetCommandName() const
{
    return TEXT("delete_datatable_row");
}

bool FDeleteDataTableRowCommand::ValidateParams(const FString& Parameters) const
{
    FString DataTableName;
    FString RowName;
    FString ParseError;
    
    return ParseParameters(Parameters, DataTableName, RowName, ParseError);
}

bool FDeleteDataTableRowCommand::ParseParameters(const FString& JsonString, FString& OutDataTableName, FString& OutRowName, FString& OutError) const
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
    
    // Parse required row_name parameter
    if (!JsonObject->TryGetStringField(TEXT("row_name"), OutRowName))
    {
        OutError = TEXT("Missing required 'row_name' parameter");
        return false;
    }
    
    return true;
}

FString FDeleteDataTableRowCommand::CreateSuccessResponse(const FString& DeletedRow) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), true);
    ResponseObj->SetStringField(TEXT("deleted_row"), DeletedRow);
    ResponseObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Successfully deleted row '%s'"), *DeletedRow));
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
    
    return OutputString;
}

FString FDeleteDataTableRowCommand::CreateErrorResponse(const FString& ErrorMessage) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), false);
    ResponseObj->SetStringField(TEXT("error"), ErrorMessage);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResponseObj.ToSharedRef(), Writer);
    
    return OutputString;
}