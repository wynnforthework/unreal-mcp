#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_MCPForEach.generated.h"

UCLASS()
class UNREALMCP_API UK2Node_MCPForEach : public UK2Node
{
    GENERATED_BODY()

public:
    UK2Node_MCPForEach(const FObjectInitializer& ObjectInitializer);

    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetTooltipText() const override;
    virtual FText GetMenuCategory() const override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

    // Pins: Array input, Element output, Exec in/out, Loop body, Array index
    UPROPERTY(EditAnywhere, Category="ForEach")
    FString ArrayPinName = TEXT("Array");
    UPROPERTY(EditAnywhere, Category="ForEach")
    FString ElementPinName = TEXT("Element");
}; 