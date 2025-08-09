// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Weapon/HandleComponent.h"

AMAPlayerCharacter::AMAPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritYaw = false;    

	Cam = CreateDefaultSubobject<UCameraComponent>("Cam");
	Cam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	Weapon = CreateDefaultSubobject<UHandleComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh()); // 필요시 소켓 지정: , TEXT("Hand_R_Socket")
	UHandleComponent::CreateDefaultPartsForOwner(this, Weapon, Weapon);
}

void AMAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector LookDir;
	if (GetLookDirectionToMouse(LookDir))
	{
		SetActorRotation(FRotator(0.f, LookDir.Rotation().Yaw, 0.f));
		UpdateCameraLead(LookDir);
	}
}

void AMAPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		OwningPlayerController->bShowMouseCursor = true;
		OwningPlayerController->bEnableClickEvents = true;
		OwningPlayerController->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 자유롭게
		InputMode.SetHideCursorDuringCapture(false); // 클릭 중에도 마우스 보임
		OwningPlayerController->SetInputMode(InputMode);
		
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem->AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

void AMAPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AMAPlayerCharacter::HandleMoveInput);
		EnhancedInputComp->BindAction(AttackInputAction, ETriggerEvent::Triggered, this, &AMAPlayerCharacter::HandleAttackInput);
		EnhancedInputComp->BindAction(SkillInputAction, ETriggerEvent::Triggered, this, &AMAPlayerCharacter::HandleSkillInput);
	}
}

FVector AMAPlayerCharacter::GetMoveForwardDir() const
{
	FVector Forward = Cam->GetForwardVector();
	Forward.Z = 0;
	Forward.Normalize();
	return Forward;
}

FVector AMAPlayerCharacter::GetMoveRightDir() const
{
	FVector Right = Cam->GetRightVector();
	Right.Z = 0;
	Right.Normalize();
	return Right;
}

void AMAPlayerCharacter::HandleMoveInput(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	if (InputVal.IsNearlyZero()) return;

	InputVal.Normalize();

	AddMovementInput(GetMoveForwardDir() * InputVal.Y + GetMoveRightDir() * InputVal.X);
}

void AMAPlayerCharacter::HandleAttackInput(const FInputActionValue& InputActionValue)
{
	const bool bPressed = InputActionValue.Get<bool>();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (bPressed)
		ASC->AbilityLocalInputPressed(static_cast<int32>(EMAAbilityInputID::Attack));
	else
		ASC->AbilityLocalInputReleased(static_cast<int32>(EMAAbilityInputID::Attack));
}

void AMAPlayerCharacter::HandleSkillInput(const FInputActionValue& InputActionValue)
{
	const bool bPressed = InputActionValue.Get<bool>();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (bPressed)
		ASC->AbilityLocalInputPressed(static_cast<int32>(EMAAbilityInputID::Skill));
	else
		ASC->AbilityLocalInputReleased(static_cast<int32>(EMAAbilityInputID::Skill));
}

bool AMAPlayerCharacter::GetLookDirectionToMouse(FVector& OutDirection) const
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return false;

	FVector WorldOrigin, WorldDir;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDir)) return false;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, WorldOrigin + WorldDir * 10000.f, ECC_Visibility, Params))
		return false;

	FVector PlayerLoc = GetActorLocation();
	FVector MouseLoc = FVector(Hit.Location.X, Hit.Location.Y, PlayerLoc.Z);
	
	FVector Dir = (MouseLoc - PlayerLoc).GetSafeNormal();
	if (Dir.IsNearlyZero()) return false;

	OutDirection = Dir;
	return true;
}

void AMAPlayerCharacter::UpdateCameraLead(const FVector& LookDirection) const
{
	if (!CameraBoom) return;
	if (LookDirection.IsNearlyZero()) return;

	FVector PlayerLoc = GetActorLocation();
	FVector LeadOffset = LookDirection * 30.f;
	FVector CamOffset = FVector(-100.f, -100.f, 0.f);

	CameraBoom->SetWorldLocation(PlayerLoc + LeadOffset + CamOffset);
}



