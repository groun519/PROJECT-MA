// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"

AMAPlayerCharacter::AMAPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = false;

	Cam = CreateDefaultSubobject<UCameraComponent>("Cam");
	Cam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

void AMAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLookAtMouse();
}

void AMAPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
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
	
}

void AMAPlayerCharacter::HandleSkillInput(const FInputActionValue& InputActionValue)
{
	
}

void AMAPlayerCharacter::UpdateLookAtMouse()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FVector WorldOrigin, WorldDirection;
	if (PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		// 라인트레이스로 바닥 클릭 지점 찾기
		FVector TraceStart = WorldOrigin;
		FVector TraceEnd = WorldOrigin + WorldDirection * 10000.f;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			FVector TargetLocation = Hit.Location;
			FVector MyLocation = GetActorLocation();

			// Z값 일치시켜서 수평 방향만 비교
			TargetLocation.Z = MyLocation.Z;

			FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();

			if (!Direction.IsNearlyZero())
			{
				FRotator LookAtRotation = Direction.Rotation();
				// Yaw만 회전 적용 (Pitch/Roll 제거)
				SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
			}
		}
	}
}


