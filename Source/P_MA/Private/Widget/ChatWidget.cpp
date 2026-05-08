#include "Widget/ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Framework/Application/SlateApplication.h"
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

void UChatWidget::SetChatFocus()
{
	if (ChatInputBox)
	{
		// 🚨 언리얼의 고질적인 프레임 포커스 씹힘 방지 (0.05초 뒤에 포커스 줌)
		FTimerHandle FocusTimer;
		GetWorld()->GetTimerManager().SetTimer(FocusTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ChatInputBox)
			{
				ChatInputBox->SetKeyboardFocus();
			}
		}), 0.05f, false);
	}
}
// 2. 포커스 돌려주기 (채팅 다 치고 다시 엔터 눌렀을 때)
void UChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 엔터를 눌러서 커밋된 경우에만 실행
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

		// 🔥 핵심: 텍스트 박스의 포커스를 명시적으로 해제합니다.
		ChatInputBox->SetIsEnabled(false);
		ChatInputBox->SetIsEnabled(true);

		if (APlayerController* PC = GetOwningPlayer())
		{
			// 1. 입력 모드를 GameOnly로 명시적 전환 (마우스 설정은 유지됨)
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);

			// 2. Slate 시스템에 키보드 제어권을 게임 뷰포트로 즉시 반환 명령
			if (FSlateApplication::IsInitialized())
			{
				FSlateApplication::Get().SetAllUserFocusToGameViewport();
			}
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