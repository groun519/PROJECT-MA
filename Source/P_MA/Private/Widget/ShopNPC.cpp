// Fill out your copyright notice in the Description page of Project Settings.

#include "ShopNPC.h"
#include "Widget/ShopWidget.h"       
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"

AShopNPC::AShopNPC()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bReplicates = true;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    
    MeshComp->SetCollisionProfileName(TEXT("CharacterMesh")); 
    MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AShopNPC::BeginPlay()
{
    Super::BeginPlay();
}

void AShopNPC::NotifyActorOnClicked(FKey ButtonPressed)
{
    Super::NotifyActorOnClicked(ButtonPressed);
    
    if (ButtonPressed != EKeys::LeftMouseButton) return;
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    
    if (PC && PC->IsLocalController())
    {
        float Distance = GetDistanceTo(PC->GetPawn());
        if (Distance > 500.f) 
        {
           return;
        }

        OpenShop(PC);
    }
}

void AShopNPC::OpenShop(APlayerController* PlayerController)
{
    if (!PlayerController || !PlayerController->IsLocalController() || !ShopWidgetClass) return;
    
    if (ActiveShopWidget && ActiveShopWidget->IsInViewport()) return;
    
    ActiveShopWidget = CreateWidget<UShopWidget>(PlayerController, ShopWidgetClass);
    
    if (ActiveShopWidget)
    {
       ActiveShopWidget->InitShop(ShopDataTables);
        
       ActiveShopWidget->AddToViewport();
        
       FInputModeGameAndUI InputMode;
       InputMode.SetWidgetToFocus(ActiveShopWidget->TakeWidget());
       InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
       PlayerController->SetInputMode(InputMode);
        
       PlayerController->SetShowMouseCursor(true);
    }
}