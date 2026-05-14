#include "MAPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "Inventory/SkillBookComponent.h"
#include "Inventory/InventoryComponent.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Weapon/WeaponComponent.h"
#include "PaperSpriteComponent.h"
#include "Player/Components/ReadyStateComponent.h"
#include "Player/Components/ReadyRideComponent.h"
#include "Player/Components/ReadyCheckWidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Convenience/InteractComponent.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "P_MA/P_MA.h"
#include "Animation/MAAnimInstance.h"
#include "Player/MAPlayerState.h"
#include "Player/Components/MAPlayerCharacterMovementComponent.h"
#include "Player/Loadout/LoadoutComponent.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutEyeShapePresetData.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Player/Mount/Data/MountData.h"
#include "Engine/DataTable.h"

AMAPlayerCharacter::AMAPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMAPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	/** Camera Set **/
	// 1) CameraBoom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("Camera Boom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritYaw = false;    
	CameraBoom->bDoCollisionTest = false;
	// 2) Cam
	Cam = CreateDefaultSubobject<UCameraComponent>("Cam");
	Cam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	/** Controller Set **/
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	// Block each other but prevent pawn-vs-pawn push/depenetration jitter.
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCharacterMovement()->InitialPushForceFactor = 0.f;
	GetCharacterMovement()->PushForceFactor = 0.f;
	GetCharacterMovement()->RepulsionForce = 0.f;
	GetCharacterMovement()->MaxDepenetrationWithPawn = 8.f;
	GetCharacterMovement()->MaxDepenetrationWithPawnAsProxy = 4.f;

	PlayerAttributeSet = CreateDefaultSubobject<UMAPlayerAttributeSet>("Player Attribute Set");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
	SkillModuleInventoryComponent = CreateDefaultSubobject<UMASkillModuleInventoryComponent>("SkillModuleInventoryComponent");

	SkillBookComponent = CreateDefaultSubobject<USkillBookComponent>(TEXT("SkillBookComponent"));

	/** Create SKCs **/
	// Create and Attach Weapon
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapon"));
	WeaponComponent->SetupAttachment(GetMesh(), TEXT("WeaponHandSocket"));

	/** Mount **/
	MountMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MountMesh"));
	MountMesh->SetupAttachment(RootComponent);
	MountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MountMesh->SetGenerateOverlapEvents(false);
	MountMesh->SetCanEverAffectNavigation(false);
	MountMesh->SetHiddenInGame(true);

    /** Mini Map **/
    // 스프라이트부터 먼저 생성
    MinimapSprite = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("MinimapSprite"));
    if (MinimapSprite)
    {
        MinimapSprite->SetupAttachment(GetMesh());
        MinimapSprite->SetCanEverAffectNavigation(false);
    }

    MinimapCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArmComp"));
    MinimapCameraBoom->SetupAttachment(RootComponent);
    MinimapCameraBoom->SetAbsolute(false, true, false);
    MinimapCameraBoom->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
    MinimapCameraBoom->TargetArmLength = 2000.0f;
    MinimapCameraBoom->bUsePawnControlRotation = false;
    MinimapCameraBoom->bInheritPitch = false;
    MinimapCameraBoom->bInheritRoll = false;
    MinimapCameraBoom->bInheritYaw = false;

    // 2. 캡처 컴포넌트 생성 및 설정
    MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureMinimap"));
    if (MinimapCapture)
    {
        MinimapCapture->SetupAttachment(MinimapCameraBoom);
        MinimapCapture->ProjectionType = ECameraProjectionMode::Orthographic;
        MinimapCapture->OrthoWidth = 7000.0f;
    	
        if (MinimapSprite)
        {
            MinimapCapture->ShowOnlyComponents.Add(MinimapSprite);
        }
    }

	static ConstructorHelpers::FObjectFinder<UCanvasRenderTarget2D> renderObj(TEXT("/Game/_Widget/Gameplay/MiniMap/CRT_MiniMap.CRT_MiniMap"));
	if (renderObj.Succeeded())
	{
		MinimapCapture->TextureTarget = renderObj.Object;
	}
	
	/** Capsule Collision **/
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Hitbox,	ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_ReadyWall, ECR_Overlap);
	
	/** Tag Init **/
	
	/** Ready State&Ride Component **/
	ReadyStateComponent = CreateDefaultSubobject<UReadyStateComponent>(TEXT("ReadyStateComponent"));
	ReadyRideComponent = CreateDefaultSubobject<UReadyRideComponent>(TEXT("ReadyRideComponent"));

	/** Ready Check Widget **/
	ReadyCheckWidget = CreateDefaultSubobject<UReadyCheckWidgetComponent>(TEXT("ReadyCheckWidget"));
	ReadyCheckWidget->SetupAttachment(GetMesh());
	ReadyCheckWidget->SetWidgetSpace(EWidgetSpace::Screen);
	ReadyCheckWidget->SetDrawAtDesiredSize(true);
	ReadyCheckWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReadyCheckWidget->SetHiddenInGame(true);
	ReadyCheckWidget->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
}

void AMAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeMinimapCapture();
	BindLoadoutDelegates();
}

void AMAPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeMinimapCapture();
	BindLoadoutDelegates();
}

void AMAPlayerCharacter::BaseChange()
{
	Super::BaseChange();
	if (ReadyRideComponent)
	{
		ReadyRideComponent->HandleOwnerBaseChanged();
	}
}

void AMAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	UpdateRotationByReadyRide(DeltaTime);
	TickMinimapCapture(DeltaTime);
	TickHeldAbilityInputs();
}

/** Player Rotate **/
void AMAPlayerCharacter::UpdateRotationByReadyRide(float DeltaTime)
{
	if (IsInputBlocked()) return;
	if (!IsRotationBlocked())
	{
		// Mouse-deproject/trace is only meaningful for the locally controlled pawn.
		if (!IsLocallyControlled()) return;

		FVector LookDir;
		if (GetLookDirectionToMouse(LookDir))
		{
			const FRotator CurrentRotation = GetActorRotation();
			const FRotator TargetRotation = FRotator(0.f, LookDir.Rotation().Yaw, 0.f);
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
			SetActorRotation(NewRotation);

			TrySendRotationToServer(LookDir);
		}
		return;
	}

	float AttachedYaw = 0.f;
	// Simulated proxies must follow replicated rotation only.
	if ((HasAuthority() || IsLocallyControlled()) &&
		ReadyRideComponent && ReadyRideComponent->TryGetRideYaw(AttachedYaw))
	{
		SetActorRotation(FRotator(0.f, AttachedYaw, 0.f));
	}
}

bool AMAPlayerCharacter::IsRotationBlocked() const
{
	if (ReadyRideComponent && ReadyRideComponent->IsRideRotationLocked()) return true;
	if (IsDead()) return true;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC && ASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetRotationLockTag());
}

bool AMAPlayerCharacter::IsInputBlocked() const
{
	if (IsDead()) return true;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC && ASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
}

void AMAPlayerCharacter::TrySendRotationToServer(const FVector& LookDirection)
{
	if (HasAuthority() || !IsLocallyControlled()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const float TargetYaw = LookDirection.Rotation().Yaw;
	const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(LastSentRotationYaw, TargetYaw));
	const float Now = World->GetTimeSeconds();
	const bool bIntervalPassed = (Now - LastRotationNetSendTime) >= RotationNetSendInterval;
	const bool bYawChangedEnough = !bHasSentRotationYaw || (YawDelta >= RotationNetSendYawThreshold);

	if (!bIntervalPassed || !bYawChangedEnough) return;

	Server_SetRotation(LookDirection);
	LastRotationNetSendTime = Now;
	LastSentRotationYaw = TargetYaw;
	bHasSentRotationYaw = true;
}

void AMAPlayerCharacter::Server_SetRotation_Implementation(FVector LookDirection)
{
	if (IsRotationBlocked()) return;
	SetActorRotation(FRotator(0.f, LookDirection.Rotation().Yaw, 0.f));
}

void AMAPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	InitializeMinimapCapture();

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

UInputAction* AMAPlayerCharacter::GetGameplayAbilityInputAction(EMAAbilityInputID InputID) const
{
	if (UInputAction* const* FoundAction = GameplayAbilityInputActions.Find(InputID)) return *FoundAction;
	return nullptr;
}

void AMAPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindLoadoutDelegates();
}

void AMAPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AMAPlayerCharacter::HandleMoveInput);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &AMAPlayerCharacter::HandleMoveInput);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Canceled, this, &AMAPlayerCharacter::HandleMoveInput);
		EnhancedInputComp->BindAction(InteractInputAction, ETriggerEvent::Started, this, &AMAPlayerCharacter::HandleInteractInput);
		
		for (const TPair<EMAAbilityInputID, UInputAction*> InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &AMAPlayerCharacter::HandleAbilityInputStarted, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &AMAPlayerCharacter::HandleAbilityInputReleased, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Canceled, this, &AMAPlayerCharacter::HandleAbilityInputReleased, InputActionPair.Key);
		}
		EnhancedInputComp->BindAction(UseInventoryItemAction, ETriggerEvent::Started, this, &AMAPlayerCharacter::UseInventoryItem);
	}
}
// 스킬 행동 로직 변형 시스템 테스트용	- 사용 법 SetSkillBehavior [BP이름] [태그]
void AMAPlayerCharacter::SetAttribute(const FString& SkillClassName, const FString& AttributeName)
{
	Server_SetAttribute(SkillClassName, AttributeName);
}

void AMAPlayerCharacter::Server_SetAttribute_Implementation(const FString& SkillClassName,
                                                                 const FString& AttributeName)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillClass = FindObject<UClass>(ANY_PACKAGE, *("GA_"+SkillClassName + "_C"));
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

void AMAPlayerCharacter::SetBehavior(const FString& SkillClassName, const FString& BehaviorTagString)
{
	Server_SetBehavior(SkillClassName, BehaviorTagString);
}
void AMAPlayerCharacter::Server_SetBehavior_Implementation(const FString& SkillClassName,
                                                                const FString& BehaviorTagString)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillClass = FindObject<UClass>(ANY_PACKAGE, *("GA_"+SkillClassName + "_C"));
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

void AMAPlayerCharacter::SetUtility(const FString& SkillClassName, const FString& UtilityName)
{
	Server_SetUtility(SkillClassName, UtilityName);
}

void AMAPlayerCharacter::Server_SetUtility_Implementation(const FString& SkillClassName, const FString& UtilityName)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	TSubclassOf<UGameplayAbility> SkillClass = FindObject<UClass>(ANY_PACKAGE, *("GA_"+SkillClassName + "_C"));
	if (!SkillClass) return;

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(SkillClass);
	if (!AbilitySpec) return;
	
	FGameplayTag BehaviorCategoryTag = FGameplayTag::RequestGameplayTag(FName("Module.Utility"));
	AbilitySpec->DynamicAbilityTags.RemoveTags(AbilitySpec->DynamicAbilityTags.Filter(FGameplayTagContainer(BehaviorCategoryTag)));
	
	FGameplayTag NewBehaviorTag = FGameplayTag::RequestGameplayTag(FName(*UtilityName));
	if (NewBehaviorTag.IsValid() && !UtilityName.Equals("None", ESearchCase::IgnoreCase))
	{
		AbilitySpec->DynamicAbilityTags.AddTag(NewBehaviorTag);
	}
	
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
	if (IsInputBlocked() || IsMovementBlocked())
	{
		RideHorizontalInput = 0.f;
		return;
	}

	RideHorizontalInput = FMath::Clamp(InputVal.X, -1.f, 1.f);
	if (InputVal.IsNearlyZero()) return;

	InputVal.Normalize();

	AddMovementInput(GetMoveForwardDir() * InputVal.Y + GetMoveRightDir() * InputVal.X);
}

void AMAPlayerCharacter::HandleInteractInput(const FInputActionValue& InputActionValue)
{
	const bool bPressed = InputActionValue.Get<bool>();
	if (!bPressed) return;
	if (IsInputBlocked()) return;

	if (UInteractComponent* Comp = CurrentInteractComp.Get())
	{
		Comp->RequestInteract(this);
	}
}

void AMAPlayerCharacter::HandleAbilityInputStarted(const FInputActionValue& InputActionValue, EMAAbilityInputID InputID)
{
	if (!InputActionValue.Get<bool>()) return;
	if (IsInputBlocked()) return;

	SetAbilityInputHeld(InputID, true);
	TryActivateHeldAbilityInput(InputID);
}

void AMAPlayerCharacter::HandleAbilityInputReleased(const FInputActionValue& /*InputActionValue*/, EMAAbilityInputID InputID)
{
	SetAbilityInputHeld(InputID, false);
}

void AMAPlayerCharacter::TickHeldAbilityInputs()
{
	if (IsInputBlocked()) return;

	for (const EMAAbilityInputID InputID : HeldAbilityInputIDs)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
		TryActivateHeldAbilityInput(InputID);
	}
}

void AMAPlayerCharacter::SetAbilityInputHeld(EMAAbilityInputID InputID, bool bHeld)
{
	if (bHeld)
	{
		HeldAbilityInputIDs.Add(InputID);
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)InputID);
		return;
	}

	HeldAbilityInputIDs.Remove(InputID);
	GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)InputID);
}

void AMAPlayerCharacter::TryActivateHeldAbilityInput(EMAAbilityInputID InputID)
{
	if (UMAAbilitySystemComponent* AbilitySystemComponent = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AbilitySystemComponent->TryActivateAbilitiesByInputID(InputID);
	}
}

void AMAPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!PlayerController)	return;

	if (bEnabled)
	{
		EnableInput(PlayerController);
	}
	else
	{
		DisableInput(PlayerController);
	}
}

void AMAPlayerCharacter::SnapRotationToMouse()
{
	if (IsInputBlocked()) return;
	if (IsRotationBlocked()) return;
	FVector LookDir;
	if (GetLookDirectionToMouse(LookDir))
	{
		SetActorRotation(FRotator(0, LookDir.Rotation().Yaw, 0));
		TrySendRotationToServer(LookDir);
	}
}

void AMAPlayerCharacter::InitializeMinimapCapture()
{
	if (!MinimapCapture) return;

	const bool bEnableCapture = IsLocallyControlled();
	MinimapCapture->SetComponentTickEnabled(false);
	MinimapCapture->bCaptureEveryFrame = false;
	MinimapCapture->bCaptureOnMovement = false;
	MinimapCaptureAccumulatedTime = 0.f;

	if (bEnableCapture) MinimapCapture->CaptureScene();
}

void AMAPlayerCharacter::TickMinimapCapture(float DeltaTime)
{
	if (!MinimapCapture || !IsLocallyControlled()) return;

	MinimapCaptureAccumulatedTime += DeltaTime;
	if (MinimapCaptureAccumulatedTime < MinimapCaptureInterval) return;

	MinimapCaptureAccumulatedTime = 0.f;
	MinimapCapture->CaptureScene();
}

void AMAPlayerCharacter::SetCurrentInteractComp(UInteractComponent* NewComp)
{
	if (!NewComp || CurrentInteractComp == NewComp) return;

	if (CurrentInteractComp.IsValid())
		CurrentInteractComp->SetInteractFocused(this, false);
	
	CurrentInteractComp = NewComp;
}

void AMAPlayerCharacter::ClearCurrentInteractComp(UInteractComponent* Comp)
{
	if (CurrentInteractComp.Get() != Comp) return;

	if (Comp) Comp->SetInteractFocused(this, false);
	CurrentInteractComp = nullptr;
}

void AMAPlayerCharacter::BindLoadoutDelegates()
{
	AMAPlayerState* NewPlayerState = GetPlayerState<AMAPlayerState>();
	if (CachedLoadoutPlayerState.Get() == NewPlayerState && NewPlayerState)
	{
		ApplyLoadoutFromPlayerState();
		return;
	}

	if (CachedLoadoutPlayerState)
	{
		if (LoadoutChangedHandle.IsValid())
		{
			CachedLoadoutPlayerState->OnLoadoutChanged.Remove(LoadoutChangedHandle);
			LoadoutChangedHandle.Reset();
		}
	}

	CachedLoadoutPlayerState = NewPlayerState;
	if (!NewPlayerState) return;

	LoadoutChangedHandle = NewPlayerState->OnLoadoutChanged.AddUObject(this, &AMAPlayerCharacter::HandleLoadoutChanged);

	ApplyLoadoutFromPlayerState();
}

void AMAPlayerCharacter::ApplyLoadoutFromPlayerState()
{
	if (!CachedLoadoutPlayerState) return;

	HandleLoadoutChanged(CachedLoadoutPlayerState->GetLoadoutSelection());
}

void AMAPlayerCharacter::HandleLoadoutChanged(const FLoadoutSelection& Loadout)
{
	HandleLoadoutColorChanged(Loadout.Color);
	HandleLoadoutEyeShapeChanged(Loadout.EyeShapeId);
	HandleLoadoutWeaponChanged(Loadout.WeaponId);
	HandleLoadoutMountChanged(Loadout.MountId);
}

void AMAPlayerCharacter::HandleLoadoutColorChanged(const FMaterialParamDataPair& ColorData)
{
	if (!LoadoutComponent) return;

	if (HasAuthority())
	{
		LoadoutComponent->SetMaterialParams(ColorData.BodyData, ColorData.EyeData);
	}
	else
	{
		LoadoutComponent->ApplyMaterialParamsLocal(ColorData);
	}
}

void AMAPlayerCharacter::HandleLoadoutEyeShapeChanged(FName EyeShapeId)
{
	if (!LoadoutComponent) return;

	const UDataTable* ResolvedEyeShapeDataTable = nullptr;
	if (const ULoadoutDataSet* LoadoutDataSet = LoadoutComponent->GetLoadoutDataSet())
	{
		ResolvedEyeShapeDataTable = LoadoutDataSet->EyeShapeDataTable;
	}

	FEyeShapeParamData EyeShapeData;
	if (!LoadoutEyeShapeTableUtils::ResolveEyeShapeData(ResolvedEyeShapeDataTable, EyeShapeId, EyeShapeData))
	{
		EyeShapeData = FEyeShapeParamData();
	}

	LoadoutComponent->ApplyEyeShapeParamsLocal(EyeShapeData);
}

void AMAPlayerCharacter::HandleLoadoutWeaponChanged(FName WeaponId)
{
	const FLoadoutWeaponDataRow* WeaponDataRow = nullptr;

	const UDataTable* ResolvedWeaponDataTable = nullptr;
	if (LoadoutComponent)
	{
		if (const ULoadoutDataSet* LoadoutDataSet = LoadoutComponent->GetLoadoutDataSet())
		{
			ResolvedWeaponDataTable = LoadoutDataSet->WeaponDataTable;
		}
	}

	if (ResolvedWeaponDataTable && !WeaponId.IsNone())
	{
		WeaponDataRow = ResolvedWeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("LoadoutWeapon"));
	}

	if (UMASkillManagerComponent* SkillManager = GetSkillManagerComponent())
	{
		UMASkillDefinition* AttackSkillDefinition = WeaponDataRow ? WeaponDataRow->AttackSkillDefinition.LoadSynchronous() : nullptr;
		if (!bHasSeededAttackSkillDefinition && AttackSkillDefinition)
		{
			bHasSeededAttackSkillDefinition = SkillManager->AddDefinition(EMAAbilityInputID::Attack, AttackSkillDefinition);
		}
	}

	if (!WeaponDataRow) return;

	USkeletalMesh* WeaponMesh = WeaponDataRow->WeaponMesh.LoadSynchronous();
	if (WeaponMesh)
	{
		WeaponComponent->SetSkeletalMesh(WeaponMesh);
	}

	WeaponComponent->SetRelativeTransform(WeaponDataRow->WeaponOffset);
}

void AMAPlayerCharacter::HandleLoadoutMountChanged(FName MountId)
{
	UAnimSequence* RiderSequence = nullptr;
	USkeletalMesh* MountSkeletalMesh = nullptr;
	TSubclassOf<UAnimInstance> MountAnimClass = nullptr;

	if (!MountId.IsNone() && LoadoutComponent)
	{
		const ULoadoutDataSet* LoadoutDataSet = LoadoutComponent->GetLoadoutDataSet();
		const UDataTable* MountDataTable = LoadoutDataSet ? LoadoutDataSet->MountDataTable : nullptr;
		if (MountDataTable)
		{
			if (const FMountDataRow* Row = MountDataTable->FindRow<FMountDataRow>(MountId, TEXT("LoadoutMount")))
			{
				MountSkeletalMesh = Row->MountMesh.LoadSynchronous();
				MountAnimClass = Row->MountAnimClass;
				RiderSequence = Row->RiderPose.LoadSynchronous();
			}
		}
	}

	MountMesh->SetSkeletalMesh(MountSkeletalMesh);
	MountMesh->SetAnimInstanceClass(MountAnimClass);

	if (UMAAnimInstance* MAAnim = Cast<UMAAnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
	{
		MAAnim->SetCurrentRideSequence(RiderSequence);
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

void AMAPlayerCharacter::OnDead()
{
	GetWorldTimerManager().ClearTimer(RespawnInputEnableTimerHandle);
	SetInputEnabledFromPlayerController(false);
	if (LoadoutComponent)
	{
		LoadoutComponent->ApplyMaterialParam(LoadoutComponent->GetMaterialParamValue(), DeadColorSaturationScale);
	}
}

void AMAPlayerCharacter::OnRespawn()
{
	bool bDeferredInputEnable = false;

	if (RespawnMontage)
	{
		const float MontageDuration = PlayAnimMontage(RespawnMontage);
		if (MontageDuration > 0.f)
		{
			bDeferredInputEnable = true;
			GetWorldTimerManager().ClearTimer(RespawnInputEnableTimerHandle);
			GetWorldTimerManager().SetTimer(
				RespawnInputEnableTimerHandle,
				this,
				&AMAPlayerCharacter::EnableInputAfterRespawnMontage,
				MontageDuration,
				false);
		}
	}

	if (!bDeferredInputEnable)
	{
		EnableInputAfterRespawnMontage();
	}

	if (LoadoutComponent)
	{
		LoadoutComponent->ApplyMaterialParam(LoadoutComponent->GetMaterialParamValue());
	}

	if (HasAuthority() && RespawnVFX)
	{
		Multicast_PlayNiagara(RespawnVFX, GetActorTransform());
	}
}

void AMAPlayerCharacter::EnableInputAfterRespawnMontage()
{
	SetInputEnabledFromPlayerController(true);
}

void AMAPlayerCharacter::UseInventoryItem(const FInputActionValue& InputActionValue)
{
	if (IsInputBlocked()) return;
	int Value = FMath::RoundToInt(InputActionValue.Get<float>());
	InventoryComponent->TryActivateItemInSlot(Value-1);
}

void AMAPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
