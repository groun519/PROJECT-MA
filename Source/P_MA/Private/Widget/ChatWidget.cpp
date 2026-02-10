#include "Widget/ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Player/MAPlayerController.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UChatWidget::OnChatTextCommitted);
	}
	
	if (AMAPlayerController* PC = Cast<AMAPlayerController>(GetOwningPlayer()))
	{
		PC->OnChatMessageReceived.AddDynamic(this, &UChatWidget::HandleChatMessageReceived);
	}
}

void UChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString Msg = Text.ToString();
		if (!Msg.IsEmpty())
		{
			if (AMAPlayerController* PC = Cast<AMAPlayerController>(GetOwningPlayer()))
			{
				PC->Server_SendChatMessage(Msg, CurrentChatType);
			}
			ChatInputBox->SetText(FText::GetEmpty());
		}
	}
}

void UChatWidget::HandleChatMessageReceived(const FString& SenderName, const FString& Message, EChatType ChatType)
{
	AddMessageToUI(SenderName, Message, ChatType);
}

void UChatWidget::AddMessageToUI(const FString& SenderName, const FString& Message, EChatType ChatType)
{
	if (!ChatScrollBox) return;

	UTextBlock* NewText = NewObject<UTextBlock>(ChatScrollBox);
	if (NewText)
	{
		FString FinalMsg;
		FSlateColor Color = FSlateColor(FLinearColor::White);
		
		switch (ChatType)
		{
		case EChatType::Normal:
			FinalMsg = FString::Printf(TEXT("%s : %s"), *SenderName, *Message);
			Color = FSlateColor(FLinearColor::White);
			break;
		
		case EChatType::System:
			FinalMsg = FString::Printf(TEXT("[System] : %s"), *Message);
			Color = FSlateColor(FLinearColor::Yellow);
			break;
		}

		NewText->SetText(FText::FromString(FinalMsg));
		NewText->SetColorAndOpacity(Color);
		NewText->SetAutoWrapText(true);

		ChatScrollBox->AddChild(NewText);
		ChatScrollBox->ScrollToEnd();
	}
}