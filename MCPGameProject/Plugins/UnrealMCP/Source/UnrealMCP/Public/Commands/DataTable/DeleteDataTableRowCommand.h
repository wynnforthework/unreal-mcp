#pragma once

#include "CoreMinimal.h"
#include "Commands/IUnrealMCPCommand.h"
#include "Services/IDataTableService.h"

/**
 * Command for deleting a single row from DataTable assets
 * Implements the IUnrealMCPCommand interface for standardized command execution
 */
class UNREALMCP_API FDeleteDataTableRowCommand : public IUnrealMCPCommand
{
public:
    /**
     * Constructor
     * @param InDataTableService - Reference to the DataTable service for operations
     */
    explicit FDeleteDataTableRowCommand(IDataTableService& InDataTableService);

    // IUnrealMCPCommand interface
    virtual FString Execute(const FString& Parameters) override;
    virtual FString GetCommandName() const override;
    virtual bool ValidateParams(const FString& Parameters) const override;

private:
    /** Reference to the DataTable service */
    IDataTableService& DataTableService;
    
    /**
     * Parse JSON parameters into DataTable name and row name
     * @param JsonString - JSON string containing parameters
     * @param OutDataTableName - Parsed DataTable name
     * @param OutRowName - Parsed row name to delete
     * @param OutError - Error message if parsing fails
     * @return true if parsing succeeded
     */
    bool ParseParameters(const FString& JsonString, FString& OutDataTableName, FString& OutRowName, FString& OutError) const;
    
    /**
     * Create success response JSON
     * @param DeletedRow - Name of successfully deleted row
     * @return JSON response string
     */
    FString CreateSuccessResponse(const FString& DeletedRow) const;
    
    /**
     * Create error response JSON
     * @param ErrorMessage - Error message
     * @return JSON response string
     */
    FString CreateErrorResponse(const FString& ErrorMessage) const;
};