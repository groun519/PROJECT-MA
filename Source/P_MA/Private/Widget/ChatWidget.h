#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/MAPlayerController.h" // Enum 사용을 위해 포함
#include "ChatWidget.generated.h"

class UScrollBox;
class UEditableTextBox;

UCLASS()
class UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInputBox;
	
	// 현재 채팅 모드 (기본값: Normal)
	EChatType CurrentChatType = EChatType::Normal; 

private:
	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleChatMessageReceived(const FString& SenderName, const FString& Message, EChatType ChatType);

	void AddMessageToUI(const FString& SenderName, const FString& Message, EChatType ChatType);
};