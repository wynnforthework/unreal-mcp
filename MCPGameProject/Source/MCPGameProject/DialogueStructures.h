#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueStructures.generated.h"

/**
 * Structure representing a player response option in dialogue
 */
USTRUCT(BlueprintType)
struct MCPGAMEPROJECT_API FDialogueResponse
{
    GENERATED_BODY()

    /** Text shown to the player for this response option */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText ResponseText;

    /** ID of the next dialogue node to go to when this response is selected */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextNodeID;

    /** Whether selecting this response ends the dialogue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bEndsDialogue;

    FDialogueResponse()
    {
        ResponseText = FText::GetEmpty();
        NextNodeID = TEXT("");
        bEndsDialogue = false;
    }
};

/**
 * Structure representing a single dialogue node
 * This struct inherits from FTableRowBase to be used with DataTables
 */
USTRUCT(BlueprintType)
struct MCPGAMEPROJECT_API FDialogueNode : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique identifier for this dialogue node */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NodeID;

    /** Name of the character speaking this dialogue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    /** The actual dialogue text to display */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    /** Array of available player response options */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FDialogueResponse> Responses;

    /** Whether this node ends the conversation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsEndNode;

    FDialogueNode()
    {
        NodeID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        Responses.Empty();
        bIsEndNode = false;
    }
};