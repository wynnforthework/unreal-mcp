#include "UMGAgentEditorWindow.h"

#include "EditorStyleSet.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Styling/AppStyle.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "UMGAgentEditorWindow"

void SUMGAgentEditorWindow::Construct(const FArguments& InArgs)
{
    // 初始化聊天历史
    ChatHistory = MakeShareable(new TArray<TSharedPtr<FChatMessage>>());
    
    // 创建主界面
    ChildSlot
    [
        SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(4.0f)
        [
            SNew(SVerticalBox)
            
            // 标题栏
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(SHorizontalBox)
                
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("WindowTitle", "UMG Agent - UI Generator"))
                    .Font(FAppStyle::GetFontStyle("ContentBrowser.TopBar.Font"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
                
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8, 0, 0, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("TestConnection", "Test Connection"))
                    .OnClicked(this, &SUMGAgentEditorWindow::OnTestConnectionClicked)
                    .ToolTipText(LOCTEXT("TestConnectionTooltip", "Test connection to Unreal Engine MCP bridge"))
                ]
                
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8, 0, 0, 0)
                [
                    SNew(SButton)
                    .Text(LOCTEXT("Settings", "Settings"))
                    .OnClicked(this, &SUMGAgentEditorWindow::OnSettingsClicked)
                    .ToolTipText(LOCTEXT("SettingsTooltip", "Open UMG Agent settings"))
                ]
            ]
            
            // 主内容区域
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)
                
                // 左侧配置面板
                + SSplitter::Slot()
                .Value(0.3f)
                [
                    CreateConfigPanel()
                ]
                
                // 右侧聊天面板
                + SSplitter::Slot()
                .Value(0.7f)
                [
                    CreateChatPanel()
                ]
            ]
            
            // 底部状态栏
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 8, 0, 0)
            [
                CreateStatusBar()
            ]
        ]
    ];
    
    // 添加欢迎消息
    AddChatMessage(EChatMessageType::System, 
        TEXT("Welcome to UMG Agent! 🎮\nDescribe the UI you want to create, and I'll generate the corresponding Widget Blueprint in Unreal Engine."));
    
    // 初始化连接状态
    UpdateConnectionStatus(false);
}

TSharedRef<SWidget> SUMGAgentEditorWindow::CreateConfigPanel()
{
    return SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(8.0f)
        [
            SNew(SScrollBox)
            
            + SScrollBox::Slot()
            [
                SNew(SVerticalBox)
                
                // 配置标题
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 0, 0, 8)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("ConfigTitle", "Configuration"))
                    .Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                ]
                
                // TCP连接设置
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 4)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("TCPHost", "TCP Host:"))
                ]
                
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 2, 0, 8)
                [
                    SAssignNew(TCPHostTextBox, SEditableTextBox)
                    .Text(FText::FromString(TEXT("127.0.0.1")))
                    .HintText(LOCTEXT("TCPHostHint", "Enter TCP host address"))
                ]
                
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 4)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("TCPPort", "TCP Port:"))
                ]
                
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 2, 0, 8)
                [
                    SAssignNew(TCPPortTextBox, SEditableTextBox)
                    .Text(FText::FromString(TEXT("55557")))
                    .HintText(LOCTEXT("TCPPortHint", "Enter TCP port"))
                ]
                
                // Widget路径设置
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 4)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("WidgetPath", "Widget Path:"))
                ]
                
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 2, 0, 8)
                [
                    SAssignNew(WidgetPathTextBox, SEditableTextBox)
                    .Text(FText::FromString(TEXT("/Game/Widgets")))
                    .HintText(LOCTEXT("WidgetPathHint", "Widget blueprint storage path"))
                ]
                
                // C++头文件路径
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 4)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("CppHeaderPath", "C++ Header Path:"))
                ]
                
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 2, 0, 16)
                [
                    SAssignNew(CppHeaderPathTextBox, SEditableTextBox)
                    .HintText(LOCTEXT("CppHeaderPathHint", "Optional: C++ header file path for binding validation"))
                ]
                
                // 连接状态
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 8)
                [
                    SAssignNew(ConnectionStatusText, STextBlock)
                    .Text(LOCTEXT("Disconnected", "🔴 Disconnected"))
                    .ColorAndOpacity(FLinearColor::Red)
                ]
                
                // 生成历史
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(0, 16, 0, 8)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("GenerationHistory", "Generation History"))
                    .Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
                ]
                
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SAssignNew(HistoryListView, SListView<TSharedPtr<FGenerationHistoryItem>>)
                    .ListItemsSource(&GenerationHistory)
                    .OnGenerateRow(this, &SUMGAgentEditorWindow::OnGenerateHistoryRow)
                    .SelectionMode(ESelectionMode::Single)
                ]
            ]
        ];
}

TSharedRef<SWidget> SUMGAgentEditorWindow::CreateChatPanel()
{
    return SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(8.0f)
        [
            SNew(SVerticalBox)
            
            // 聊天标题
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("ChatTitle", "UI Generation Chat"))
                .Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
            ]
            
            // 聊天显示区域
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .Padding(0, 0, 0, 8)
            [
                SNew(SBorder)
                .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .Padding(4.0f)
                [
                    SAssignNew(ChatScrollBox, SScrollBox)
                    .Orientation(Orient_Vertical)
                    .ScrollBarAlwaysVisible(true)
                ]
            ]
            
            // 输入区域
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .Padding(0, 0, 8, 0)
                [
                    SAssignNew(InputTextBox, SMultiLineEditableTextBox)
                    .HintText(LOCTEXT("InputHint", "Describe the UI you want to generate..."))
                    .AutoWrapText(true)
                    .Margin(FMargin(4.0f))
                    .OnKeyDownHandler(this, &SUMGAgentEditorWindow::OnInputKeyDown)
                ]
                
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SVerticalBox)
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 4)
                    [
                        SAssignNew(GenerateButton, SButton)
                        .Text(LOCTEXT("Generate", "Generate UI"))
                        .OnClicked(this, &SUMGAgentEditorWindow::OnGenerateUIClicked)
                        .IsEnabled(true)
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SButton)
                        .Text(LOCTEXT("Clear", "Clear Chat"))
                        .OnClicked(this, &SUMGAgentEditorWindow::OnClearChatClicked)
                    ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SUMGAgentEditorWindow::CreateStatusBar()
{
    return SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)
            
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SAssignNew(StatusText, STextBlock)
                .Text(LOCTEXT("Ready", "Ready"))
            ]
            
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(8, 0, 0, 0)
            [
                SAssignNew(ProgressText, STextBlock)
                .Text(FText::GetEmpty())
                .ColorAndOpacity(FLinearColor::Yellow)
            ]
        ];
}

void SUMGAgentEditorWindow::AddChatMessage(EChatMessageType MessageType, const FString& Message)
{
    TSharedPtr<FChatMessage> NewMessage = MakeShareable(new FChatMessage());
    NewMessage->Type = MessageType;
    NewMessage->Content = Message;
    NewMessage->Timestamp = FDateTime::Now();
    
    ChatHistory->Add(NewMessage);
    
    // 创建消息widget
    TSharedRef<SWidget> MessageWidget = CreateChatMessageWidget(NewMessage);
    
    // 添加到聊天显示区域
    ChatScrollBox->AddSlot()
    .Padding(0, 2)
    [
        MessageWidget
    ];
    
    // 滚动到底部
    ChatScrollBox->ScrollToEnd();
}

TSharedRef<SWidget> SUMGAgentEditorWindow::CreateChatMessageWidget(TSharedPtr<FChatMessage> Message)
{
    FLinearColor MessageColor = FLinearColor::White;
    FString MessagePrefix;
    
    switch (Message->Type)
    {
        case EChatMessageType::User:
            MessageColor = FLinearColor::Blue;
            MessagePrefix = TEXT("[USER] ");
            break;
        case EChatMessageType::Assistant:
            MessageColor = FLinearColor::Green;
            MessagePrefix = TEXT("[ASSISTANT] ");
            break;
        case EChatMessageType::System:
            MessageColor = FLinearColor::Gray;
            MessagePrefix = TEXT("[SYSTEM] ");
            break;
        case EChatMessageType::Error:
            MessageColor = FLinearColor::Red;
            MessagePrefix = TEXT("[ERROR] ");
            break;
    }
    
    return SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
        .Padding(8.0f)
        [
            SNew(SVerticalBox)
            
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString(MessagePrefix + Message->Content))
                .ColorAndOpacity(MessageColor)
                .AutoWrapText(true)
            ]
            
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 4, 0, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Message->Timestamp.ToString(TEXT("%H:%M:%S"))))
                .Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
                .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
            ]
        ];
}

FReply SUMGAgentEditorWindow::OnGenerateUIClicked()
{
    FString InputText = InputTextBox->GetText().ToString().TrimStartAndEnd();
    
    if (InputText.IsEmpty())
    {
        ShowNotification(LOCTEXT("EmptyInput", "Please enter a UI description"), SNotificationItem::CS_Fail);
        return FReply::Handled();
    }
    
    // 添加用户消息到聊天
    AddChatMessage(EChatMessageType::User, InputText);
    
    // 清空输入框
    InputTextBox->SetText(FText::GetEmpty());
    
    // 开始生成UI
    StartUIGeneration(InputText);
    
    return FReply::Handled();
}

void SUMGAgentEditorWindow::StartUIGeneration(const FString& Description)
{
    // 禁用生成按钮
    GenerateButton->SetEnabled(false);
    
    // 显示生成中状态
    AddChatMessage(EChatMessageType::System, TEXT("🔄 Generating UI..."));
    UpdateStatus(TEXT("Generating UI..."));
    
    // 这里应该调用实际的UI生成逻辑
    // 暂时使用模拟的异步操作
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Description]()
    {
        SimulateUIGeneration(Description);
    }, 2.0f, false);
}

void SUMGAgentEditorWindow::SimulateUIGeneration(const FString& Description)
{
    // 模拟UI生成结果
    bool bSuccess = FMath::RandBool(); // 随机成功/失败
    
    if (bSuccess)
    {
        FString ResultMessage = FString::Printf(TEXT("✅ UI generated successfully!\n\n📁 Widget Path: /Game/Widgets/GeneratedWidget_%d\n🔧 Created Components: 3\n\n💡 Recommendations:\n• Consider adding hover effects\n• Adjust button sizes for better UX"), FMath::RandRange(1000, 9999));
        
        AddChatMessage(EChatMessageType::Assistant, ResultMessage);
        
        // 添加到历史记录
        TSharedPtr<FGenerationHistoryItem> HistoryItem = MakeShareable(new FGenerationHistoryItem());
        HistoryItem->Description = Description.Left(50) + (Description.Len() > 50 ? TEXT("...") : TEXT(""));
        HistoryItem->bSuccess = true;
        HistoryItem->Timestamp = FDateTime::Now();
        HistoryItem->ComponentCount = 3;
        
        GenerationHistory.Add(HistoryItem);
        HistoryListView->RequestListRefresh();
        
        ShowNotification(LOCTEXT("GenerationSuccess", "UI generated successfully!"), SNotificationItem::CS_Success);
    }
    else
    {
        FString ErrorMessage = TEXT("❌ UI generation failed\n\nError: Connection timeout\n• Please ensure Unreal Engine is running\n• Check if UnrealMCP plugin is loaded");
        
        AddChatMessage(EChatMessageType::Error, ErrorMessage);
        
        ShowNotification(LOCTEXT("GenerationFailed", "UI generation failed"), SNotificationItem::CS_Fail);
    }
    
    // 重新启用生成按钮
    GenerateButton->SetEnabled(true);
    UpdateStatus(TEXT("Ready"));
}

FReply SUMGAgentEditorWindow::OnTestConnectionClicked()
{
    UpdateStatus(TEXT("Testing connection..."));
    
    // 模拟连接测试
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        bool bConnected = FMath::RandBool();
        UpdateConnectionStatus(bConnected);
        
        if (bConnected)
        {
            AddChatMessage(EChatMessageType::System, TEXT("✅ Connected to Unreal Engine"));
            ShowNotification(LOCTEXT("ConnectionSuccess", "Connected to Unreal Engine"), SNotificationItem::CS_Success);
        }
        else
        {
            AddChatMessage(EChatMessageType::System, TEXT("❌ Cannot connect to Unreal Engine. Please ensure UE is running with UnrealMCP plugin loaded."));
            ShowNotification(LOCTEXT("ConnectionFailed", "Connection failed"), SNotificationItem::CS_Fail);
        }
        
        UpdateStatus(TEXT("Ready"));
    }, 1.0f, false);
    
    return FReply::Handled();
}

FReply SUMGAgentEditorWindow::OnSettingsClicked()
{
    // 这里可以打开设置对话框
    ShowNotification(LOCTEXT("SettingsNotImplemented", "Settings dialog not implemented yet"), SNotificationItem::CS_Pending);
    return FReply::Handled();
}

FReply SUMGAgentEditorWindow::OnClearChatClicked()
{
    // 清空聊天历史
    ChatHistory->Empty();
    ChatScrollBox->ClearChildren();
    
    // 添加欢迎消息
    AddChatMessage(EChatMessageType::System, TEXT("Chat cleared. Welcome back to UMG Agent! 🎮"));
    
    return FReply::Handled();
}

FReply SUMGAgentEditorWindow::OnInputKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter && InKeyEvent.IsControlDown())
    {
        return OnGenerateUIClicked();
    }
    
    return FReply::Unhandled();
}

TSharedRef<ITableRow> SUMGAgentEditorWindow::OnGenerateHistoryRow(TSharedPtr<FGenerationHistoryItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FGenerationHistoryItem>>, OwnerTable)
    [
        SNew(SHorizontalBox)
        
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 4, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Item->bSuccess ? TEXT("✅") : TEXT("❌")))
        ]
        
        + SHorizontalBox::Slot()
        .FillWidth(1.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Item->Description))
            .ToolTipText(FText::FromString(FString::Printf(TEXT("Time: %s\nComponents: %d"), 
                *Item->Timestamp.ToString(), Item->ComponentCount)))
        ]
    ];
}

void SUMGAgentEditorWindow::UpdateConnectionStatus(bool bConnected)
{
    if (bConnected)
    {
        ConnectionStatusText->SetText(LOCTEXT("Connected", "🟢 Connected"));
        ConnectionStatusText->SetColorAndOpacity(FLinearColor::Green);
    }
    else
    {
        ConnectionStatusText->SetText(LOCTEXT("Disconnected", "🔴 Disconnected"));
        ConnectionStatusText->SetColorAndOpacity(FLinearColor::Red);
    }
}

void SUMGAgentEditorWindow::UpdateStatus(const FString& Status)
{
    StatusText->SetText(FText::FromString(Status));
}

void SUMGAgentEditorWindow::ShowNotification(const FText& Message, SNotificationItem::ECompletionState CompletionState)
{
    FNotificationInfo Info(Message);
    Info.bFireAndForget = true;
    Info.FadeOutDuration = 3.0f;
    Info.ExpireDuration = 5.0f;
    
    TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
    if (NotificationItem.IsValid())
    {
        NotificationItem->SetCompletionState(CompletionState);
    }
}

UWorld* SUMGAgentEditorWindow::GetWorld() const
{
    // For Slate widgets, we need to get the world from the editor context
    if (GEngine)
    {
        // Try to get the world from the editor world context
        if (UWorld* EditorWorld = GEngine->GetWorldContexts()[0].World())
        {
            return EditorWorld;
        }
        
        // Fallback: try to get any world
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (UWorld* World = Context.World())
            {
                return World;
            }
        }
    }
    
    return nullptr;
}

#undef LOCTEXT_NAMESPACE
