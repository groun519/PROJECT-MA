// Fill out your copyright notice in the Description page of Project Settings.


#include "MAPlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "PaperSpriteComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "Inventory/SkillBookComponent.h"
#include "Inventory/InventoryComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Weapon/WeaponComponent.h"
#include "DrawDebugHelpers.h"


AMAPlayerCharacter::AMAPlayerCharacter()
{
	/** Camera Set **//*
	 * 1. CameraBoom cannot use "Pawn Control Rot"
	 *		-> Because, player looks mouse pointer.
	 * 2. CameraBoom must lock Yaw
	 *		-> Because, Camera must not rotate z axis.
	 */
	// 1) CameraBoom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritYaw = false;    
	// 2) Cam
	Cam = CreateDefaultSubobject<UCameraComponent>("Cam");
	Cam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	/** Controller Set **//*
	 * 1. Player cannot use "Controller Rot"
	 *		-> Because, player cam's rot must be fixed.
	 * 2. Player cannot use "Origin Rot to Movement"
	 *		-> Because, player must look mouse pointer.
	 */
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	PlayerAttributeSet = CreateDefaultSubobject<UMAPlayerAttributeSet>("Player Attribute Set");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");

	SkillBookComponent = CreateDefaultSubobject<USkillBookComponent>(TEXT("SkillBookComponent"));
	
	/** Create SKCs **//*
	 * - Child Relationship : Mesh - Handle
	 */
	// Create and Attach Weapon
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapon"));
	WeaponComponent->SetupAttachment(GetMesh(), TEXT("WeaponHandSocket"));

	/** Mini Map 아래 코드는 공부할 필요 없음 강의 에는 없는 코드 입니다 **/
	MinimapCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArmComp"));
	MinimapCameraBoom->SetupAttachment(RootComponent);
	MinimapCameraBoom->SetWorldRotation(FRotator(-90.0f, 45.0f, 0.0f));

	MinimapCameraBoom->TargetArmLength = 900.0f;
	MinimapCameraBoom->bUsePawnControlRotation = false;
	MinimapCameraBoom->bInheritPitch = false;
	MinimapCameraBoom->bInheritRoll = false;
	MinimapCameraBoom->bInheritYaw = false;

	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureMinimap"));
	MinimapCapture->SetupAttachment(MinimapCameraBoom);
	MinimapCapture->ProjectionType = ECameraProjectionMode::Orthographic;
	MinimapCapture->OrthoWidth = 1700.0f;
	MinimapCapture->ShowOnlyComponents.Add(MinimapSprite);

	RotationLockTag=UMAAbilitySystemStatics::GetRotationLockTag();
	RushingTag=UMAAbilitySystemStatics::GetRushingTag();
	

	static ConstructorHelpers::FObjectFinder<UCanvasRenderTarget2D> renderObj(TEXT("/Game/Luco/Minimap/CRT_Minimap.CRT_Minimap"));
	if (renderObj.Succeeded())
	{
		MinimapCapture->TextureTarget = renderObj.Object;
	}
	MinimapSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MinimapSprite"));
	MinimapSprite->SetupAttachment(GetMesh());
	/** 여기 위에 까지는 별도의 코드 입니다 **/
	
}

void AMAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector LookDir;
	if (GetLookDirectionToMouse(LookDir) && !GetAbilitySystemComponent()->HasMatchingGameplayTag(RotationLockTag))
	{
		const FRotator CurrentRotation = GetActorRotation();
		const FRotator TargetRotation = FRotator(0.f, LookDir.Rotation().Yaw, 0.f);
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime,RotationInterpSpeed);
		SetActorRotation(NewRotation);
		
		if (!HasAuthority())
		{
			Server_SetRotation(LookDir);
		}
	}
	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(RushingTag))
	{
		AddMovementInput(GetActorForwardVector(), 2.f);
	}
}

void AMAPlayerCharacter::Server_SetRotation_Implementation(FVector LookDirection)
{
	SetActorRotation(FRotator(0.f, LookDirection.Rotation().Yaw, 0.f));
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
		EnhancedInputComp->BindAction(InteractInputAction, ETriggerEvent::Started, this, &AMAPlayerCharacter::HandleInteractInput);
		
		for (const TPair<EMAAbilityInputID, UInputAction*> InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &AMAPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &AMAPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Canceled, this, &AMAPlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}
		EnhancedInputComp->BindAction(UseInventoryItemAction, ETriggerEvent::Triggered, this, &AMAPlayerCharacter::UseInventoryItem);
	}
}
// 스킬 행동 로직 변형 시스템 테스트용	- 사용 법 SetSkillBehavior [BP이름] [태그]
void AMAPlayerCharacter::SetSkillAttribute(const FString& SkillClassName, const FString& AttributeName)
{
	Server_SetSkillAttribute(SkillClassName, AttributeName);
}

void AMAPlayerCharacter::Server_SetSkillAttribute_Implementation(const FString& SkillClassName,
	const FString& AttributeName)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillClass = FindObject<UClass>(ANY_PACKAGE, *("GA_"+SkillClassName + "_BP_C"));
	if (!SkillClass) return;

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(SkillClass);
	if (!AbilitySpec) return;

	FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag("Ability.Attribute");
	AbilitySpec->DynamicAbilityTags.RemoveTags(AbilitySpec->DynamicAbilityTags.Filter(FGameplayTagContainer(AttributeTag)));
	FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(FName(*AttributeName));
	if (NewTag.IsValid() && !AttributeName.Equals("None", ESearchCase::IgnoreCase))
	{
		AbilitySpec->DynamicAbilityTags.AddTag(NewTag);
	}
	ASC->MarkAbilitySpecDirty(*AbilitySpec);
}

void AMAPlayerCharacter::SetSkillBehavior(const FString& SkillClassName, const FString& BehaviorTagString)
{
	Server_SetSkillBehavior(SkillClassName, BehaviorTagString);
}
void AMAPlayerCharacter::Server_SetSkillBehavior_Implementation(const FString& SkillClassName,
                                                                const FString& BehaviorTagString)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillClass = FindObject<UClass>(ANY_PACKAGE, *("GA_"+SkillClassName + "_BP_C"));
	if (!SkillClass) return;

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(SkillClass);
	if (!AbilitySpec) return;

	// 1. 기존의 모든 Behavior 관련 태그를 제거합니다.
	FGameplayTag BehaviorCategoryTag = FGameplayTag::RequestGameplayTag(FName("Ability.Behavior"));
	AbilitySpec->DynamicAbilityTags.RemoveTags(AbilitySpec->DynamicAbilityTags.Filter(FGameplayTagContainer(BehaviorCategoryTag)));

	// 2. "None"이 아닐 경우에만 새로운 태그를 추가합니다.
	FGameplayTag NewBehaviorTag = FGameplayTag::RequestGameplayTag(FName(*BehaviorTagString));
	if (NewBehaviorTag.IsValid() && !BehaviorTagString.Equals("None", ESearchCase::IgnoreCase))
	{
		AbilitySpec->DynamicAbilityTags.AddTag(NewBehaviorTag);
	}

	// 3. 변경사항을 모든 클라이언트에 동기화합니다.
	ASC->MarkAbilitySpecDirty(*AbilitySpec);
}
//******************************************************************************//

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


void AMAPlayerCharacter::HandleInteractInput(const FInputActionValue& InputActionValue)
{
	const bool bPressed = InputActionValue.Get<bool>();
	if (!bPressed) return;
}

void AMAPlayerCharacter::HandleAbilityInput(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID)
{
	bool bPressed = InputActionValue.Get<bool>();
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
	}
	if (InputID == EMAAbilityInputID::Attack)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UMAAbilitySystemStatics::GetBasicAttackAbilityTag(),FGameplayEventData());
		Server_SendGameplayEventToSelf(UMAAbilitySystemStatics::GetBasicAttackAbilityTag(),FGameplayEventData());
	}
}

void AMAPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController)	return;

	if (bEnabled)
	{
		EnableInput(PlayerController);
	}else
	{
		DisableInput(PlayerController);
	}
}

void AMAPlayerCharacter::SnapRotationToMouse()
{
	FVector LookDir;
	if (GetLookDirectionToMouse(LookDir))
	{
		SetActorRotation(FRotator(0, LookDir.Rotation().Yaw, 0));
		if (!HasAuthority())
		{
			Server_SetRotation(LookDir);
		}
	}
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

void AMAPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void AMAPlayerCharacter::OnRecoverFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void AMAPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void AMAPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void AMAPlayerCharacter::OnGhostMode()
{
	
}

void AMAPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	int Value = FMath::RoundToInt(InputActionValue.Get<float>());
	InventoryComponent->TryActivateItemInSlot(Value-1);
}

