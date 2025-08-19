#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/DateTime.h"

// 聊天消息类型
UENUM()
enum class EChatMessageType : uint8
{
    User,
    Assistant,
    System,
    Error
};

// 聊天消息结构
struct FChatMessage
{
    EChatMessageType Type;
    FString Content;
    FDateTime Timestamp;
};

// 生成历史项目结构
struct FGenerationHistoryItem
{
    FString Description;
    bool bSuccess;
    FDateTime Timestamp;
    int32 ComponentCount;
};

/**
 * UMG Agent编辑器窗口
 * 提供可视化界面用于生成UMG Widget蓝图
 */
class UNREALMCP_API SUMGAgentEditorWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SUMGAgentEditorWindow) {}
    SLATE_END_ARGS()

    /** 构造函数 */
    void Construct(const FArguments& InArgs);

private:
    // UI创建函数
    TSharedRef<SWidget> CreateConfigPanel();
    TSharedRef<SWidget> CreateChatPanel();
    TSharedRef<SWidget> CreateStatusBar();
    TSharedRef<SWidget> CreateChatMessageWidget(TSharedPtr<FChatMessage> Message);

    // 事件处理函数
    FReply OnGenerateUIClicked();
    FReply OnTestConnectionClicked();
    FReply OnSettingsClicked();
    FReply OnClearChatClicked();
    FReply OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

    // 历史记录相关
    TSharedRef<ITableRow> OnGenerateHistoryRow(TSharedPtr<FGenerationHistoryItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

    // 聊天相关函数
    void AddChatMessage(EChatMessageType MessageType, const FString& Message);
    
    // UI生成相关函数
    void StartUIGeneration(const FString& Description);
    void SimulateUIGeneration(const FString& Description);

    // 状态更新函数
    void UpdateConnectionStatus(bool bConnected);
    void UpdateStatus(const FString& Status);
    void ShowNotification(const FText& Message, SNotificationItem::ECompletionState CompletionState);

    // 辅助函数
    UWorld* GetWorld() const;

private:
    // 配置相关控件
    TSharedPtr<SEditableTextBox> TCPHostTextBox;
    TSharedPtr<SEditableTextBox> TCPPortTextBox;
    TSharedPtr<SEditableTextBox> WidgetPathTextBox;
    TSharedPtr<SEditableTextBox> CppHeaderPathTextBox;
    TSharedPtr<STextBlock> ConnectionStatusText;

    // 聊天相关控件
    TSharedPtr<SScrollBox> ChatScrollBox;
    TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
    TSharedPtr<SButton> GenerateButton;

    // 状态栏控件
    TSharedPtr<STextBlock> StatusText;
    TSharedPtr<STextBlock> ProgressText;

    // 历史记录控件
    TSharedPtr<SListView<TSharedPtr<FGenerationHistoryItem>>> HistoryListView;

    // 数据
    TSharedPtr<TArray<TSharedPtr<FChatMessage>>> ChatHistory;
    TArray<TSharedPtr<FGenerationHistoryItem>> GenerationHistory;

    // 状态
    bool bIsConnected = false;
    bool bIsGenerating = false;
};
