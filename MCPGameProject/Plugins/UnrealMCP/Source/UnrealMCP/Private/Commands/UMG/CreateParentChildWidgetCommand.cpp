#include "Commands/UMG/CreateParentChildWidgetCommand.h"
#include "Services/UMG/IUMGService.h"
#include "MCPErrorHandler.h"
#include "MCPError.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"

FCreateParentChildWidgetCommand::FCreateParentChildWidgetCommand(TSharedPtr<IUMGService> InUMGService)
    : UMGService(InUMGService)
{
}

FString FCreateParentChildWidgetCommand::Execute(const FString& Parameters)
{
    UE_LOG(LogTemp, Warning, TEXT("=== NEW ARCHITECTURE PROOF ==="));
    UE_LOG(LogTemp, Warning, TEXT("FCreateParentChildWidgetCommand::Execute: NEW ARCHITECTURE COMMAND CALLED!"));
    UE_LOG(LogTemp, Warning, TEXT("Using service layer delegation instead of direct legacy calls"));
    
    // Parse JSON parameters
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Parameters);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        FMCPError Error = FMCPErrorHandler::CreateValidationFailedError(TEXT("Invalid JSON parameters"));
        TSharedPtr<FJsonObject> ErrorResponse = CreateErrorResponse(Error);
        
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        FJsonSerializer::Serialize(ErrorResponse.ToSharedRef(), Writer);
        return OutputString;
    }

    // Use internal execution with JSON objects
    TSharedPtr<FJsonObject> Response = ExecuteInternal(JsonObject);
    
    // Convert response back to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Response.ToSharedRef(), Writer);
    return OutputString;
}

TSharedPtr<FJsonObject> FCreateParentChildWidgetCommand::ExecuteInternal(const TSharedPtr<FJsonObject>& Params)
{
    if (!UMGService.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("NEW ARCHITECTURE: UMGService is not valid!"));
        FMCPError Error = FMCPErrorHandler::CreateInternalError(TEXT("UMG Service is not available"));
        return CreateErrorResponse(Error);
    }

    // Validate parameters using new architecture
    FString ValidationError;
    if (!ValidateParamsInternal(Params, ValidationError))
    {
        FMCPError Error = FMCPErrorHandler::CreateValidationFailedError(ValidationError);
        return CreateErrorResponse(Error);
    }

    // Extract parameters
    FString BlueprintName = Params->GetStringField(TEXT("blueprint_name"));
    FString ParentComponentName = Params->GetStringField(TEXT("parent_component_name"));
    FString ChildComponentName = Params->GetStringField(TEXT("child_component_name"));
    FString ParentComponentType = Params->GetStringField(TEXT("parent_component_type"));
    FString ChildComponentType = Params->GetStringField(TEXT("child_component_type"));
    
    FVector2D ParentPosition(0.0f, 0.0f);
    const TArray<TSharedPtr<FJsonValue>>* PositionArray;
    if (Params->TryGetArrayField(TEXT("parent_position"), PositionArray) && PositionArray->Num() >= 2)
    {
        ParentPosition.X = (*PositionArray)[0]->AsNumber();
        ParentPosition.Y = (*PositionArray)[1]->AsNumber();
    }
    
    FVector2D ParentSize(300.0f, 200.0f);
    const TArray<TSharedPtr<FJsonValue>>* SizeArray;
    if (Params->TryGetArrayField(TEXT("parent_size"), SizeArray) && SizeArray->Num() >= 2)
    {
        ParentSize.X = (*SizeArray)[0]->AsNumber();
        ParentSize.Y = (*SizeArray)[1]->AsNumber();
    }

    TSharedPtr<FJsonObject> ChildAttributes;
    const TSharedPtr<FJsonObject>* ChildAttributesObject;
    if (Params->TryGetObjectField(TEXT("child_attributes"), ChildAttributesObject))
    {
        ChildAttributes = *ChildAttributesObject;
    }
    else
    {
        ChildAttributes = MakeShared<FJsonObject>();
    }

    // Use the UMG service to create parent and child widget components
    bool bSuccess = UMGService->CreateParentAndChildWidgetComponents(BlueprintName, ParentComponentName, ChildComponentName,
                                                                    ParentComponentType, ChildComponentType, 
                                                                    ParentPosition, ParentSize, ChildAttributes);
    if (!bSuccess)
    {
        FString ErrorMessage = FString::Printf(TEXT("Failed to create parent '%s' and child '%s' widget components"), 
                                             *ParentComponentName, *ChildComponentName);
        FMCPError Error = FMCPErrorHandler::CreateExecutionFailedError(ErrorMessage);
        return CreateErrorResponse(Error);
    }

    return CreateSuccessResponse(BlueprintName, ParentComponentName, ChildComponentName);
}

FString FCreateParentChildWidgetCommand::GetCommandName() const
{
    return TEXT("create_parent_and_child_widget_components");
}

bool FCreateParentChildWidgetCommand::ValidateParams(const FString& Parameters) const
{
    // Parse JSON parameters
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Parameters);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        return false;
    }

    FString ValidationError;
    return ValidateParamsInternal(JsonObject, ValidationError);
}

bool FCreateParentChildWidgetCommand::ValidateParamsInternal(const TSharedPtr<FJsonObject>& Params, FString& OutError) const
{
    // Required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) || BlueprintName.IsEmpty())
    {
        OutError = TEXT("Missing or empty 'blueprint_name' parameter");
        return false;
    }

    FString ParentComponentName;
    if (!Params->TryGetStringField(TEXT("parent_component_name"), ParentComponentName) || ParentComponentName.IsEmpty())
    {
        OutError = TEXT("Missing or empty 'parent_component_name' parameter");
        return false;
    }

    FString ChildComponentName;
    if (!Params->TryGetStringField(TEXT("child_component_name"), ChildComponentName) || ChildComponentName.IsEmpty())
    {
        OutError = TEXT("Missing or empty 'child_component_name' parameter");
        return false;
    }

    // Optional parameters - set defaults if not provided
    if (!Params->HasField(TEXT("parent_component_type")))
    {
        Params->SetStringField(TEXT("parent_component_type"), TEXT("Border"));
    }

    if (!Params->HasField(TEXT("child_component_type")))
    {
        Params->SetStringField(TEXT("child_component_type"), TEXT("TextBlock"));
    }

    return true;
}

TSharedPtr<FJsonObject> FCreateParentChildWidgetCommand::CreateSuccessResponse(const FString& BlueprintName, const FString& ParentComponentName, const FString& ChildComponentName) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), true);
    ResponseObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
    ResponseObj->SetStringField(TEXT("parent_component_name"), ParentComponentName);
    ResponseObj->SetStringField(TEXT("child_component_name"), ChildComponentName);
    ResponseObj->SetStringField(TEXT("message"), TEXT("Parent and child widget components created successfully"));

    return ResponseObj;
}

TSharedPtr<FJsonObject> FCreateParentChildWidgetCommand::CreateErrorResponse(const FMCPError& Error) const
{
    TSharedPtr<FJsonObject> ResponseObj = MakeShared<FJsonObject>();
    ResponseObj->SetBoolField(TEXT("success"), false);
    ResponseObj->SetStringField(TEXT("error"), Error.ErrorMessage);
    ResponseObj->SetStringField(TEXT("error_details"), Error.ErrorDetails);
    ResponseObj->SetNumberField(TEXT("error_code"), Error.ErrorCode);

    return ResponseObj;
}