#include "Level/Destination/MADestinationVoteActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "AI/Data/MonstersByEnvironmentData.h"
#include "Level/Environment/EnvironmentManager.h"
#include "Net/UnrealNetwork.h"
#include "Widget/Destination/MADestinationInfoWidget.h"
#include "Widget/Destination/MADestinationVoteStatusWidget.h"

AMADestinationVoteActor::AMADestinationVoteActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
	InteractableComponent->CALL_SETUP_INTERACTION_MODE(Server);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(MeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);

	VoteStatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VoteStatusWidgetComponent"));
	VoteStatusWidgetComponent->SetupAttachment(RootComponent);
	VoteStatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	VoteStatusWidgetComponent->SetDrawAtDesiredSize(true);
	VoteStatusWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
	VoteStatusWidgetComponent->SetVisibility(false);

	DestinationInfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DestinationInfoWidgetComponent"));
	DestinationInfoWidgetComponent->SetupAttachment(RootComponent);
	DestinationInfoWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DestinationInfoWidgetComponent->SetDrawAtDesiredSize(true);
	DestinationInfoWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 240.f));
	DestinationInfoWidgetComponent->SetVisibility(false);
}

void AMADestinationVoteActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshDestinationInfoWidget();
}

void AMADestinationVoteActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMADestinationVoteActor, Voters);
	DOREPLIFETIME(AMADestinationVoteActor, OptionData);
}

bool AMADestinationVoteActor::RequestVote(AMAPlayerCharacter* Interactor)
{
	if (!HasAuthority() || !Interactor) return false;
	if (InteractableComponent && !InteractableComponent->CanServerInteract(Interactor)) return false;
	if (!OnVoteRequested.IsBound()) return false;

	OnVoteRequested.Broadcast(this, Interactor);
	return true;
}

bool AMADestinationVoteActor::ApplyVote(APlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState || Voters.Contains(PlayerState)) return false;

	Voters.Add(PlayerState);
	NotifyVoteChanged();
	ForceNetUpdate();
	return true;
}

bool AMADestinationVoteActor::RemoveVote(APlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState) return false;
	
	const int32 RemovedCount = Voters.Remove(PlayerState);
	if (RemovedCount <= 0) return false;

	NotifyVoteChanged();
	ForceNetUpdate();
	return true;
}

bool AMADestinationVoteActor::ClearVotes()
{
	if (!HasAuthority() || Voters.IsEmpty()) return false;

	Voters.Reset();
	NotifyVoteChanged();
	ForceNetUpdate();
	return true;
}

void AMADestinationVoteActor::HandleInteract(AMAPlayerCharacter* Interactor)
{
	RequestVote(Interactor);
}

void AMADestinationVoteActor::NotifyVoteChanged()
{
	if (UMADestinationVoteStatusWidget* VoteStatusWidget =
		Cast<UMADestinationVoteStatusWidget>(VoteStatusWidgetComponent->GetUserWidgetObject()))
	{
		VoteStatusWidget->SetVoters(Voters);
	}
	VoteStatusWidgetComponent->SetVisibility(!Voters.IsEmpty());
}

void AMADestinationVoteActor::OnRep_Voters()
{
	NotifyVoteChanged();
}

void AMADestinationVoteActor::OnRep_OptionData()
{
	RefreshDestinationInfoWidget();
}

void AMADestinationVoteActor::RefreshDestinationInfoWidget()
{
	if (!DestinationInfoWidgetComponent) return;
	DestinationInfoWidgetComponent->InitWidget();

	if (UMADestinationInfoWidget* InfoWidget =
		Cast<UMADestinationInfoWidget>(DestinationInfoWidgetComponent->GetUserWidgetObject()))
	{
		UTexture2D* Icon = ResolveDestinationIcon();
		InfoWidget->SetEnvIcon(Icon);
		DestinationInfoWidgetComponent->SetVisibility(Icon != nullptr);
	}
}

UTexture2D* AMADestinationVoteActor::ResolveDestinationIcon() const
{
	if (!OptionData.EnvTag.IsValid()) return nullptr;

	const AEnvironmentManager* EnvironmentManager = AEnvironmentManager::FindEnvironmentManager(GetWorld());
	const FMonstersByEnvironmentData* EnvironmentData =
		EnvironmentManager ? EnvironmentManager->FindEnvironmentData(OptionData.EnvTag) : nullptr;
	return EnvironmentData ? EnvironmentData->DestinationIcon : nullptr;
}
