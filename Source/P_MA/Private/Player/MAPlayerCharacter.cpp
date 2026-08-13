#include "MAPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Framework/MAGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "Inventory/MAInventoryComponent.h"
#include "Weapon/WeaponComponent.h"
#include "PaperSpriteComponent.h"
#include "Player/Components/ReadyStateComponent.h"
#include "Player/Components/ReadyRideComponent.h"
#include "Player/Components/ReadyCheckWidgetComponent.h"
#include "Player/Components/MACurrencyComponent.h"
#include "Components/CapsuleComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractorComponent.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "P_MA/P_MA.h"
#include "Animation/MAAnimInstance.h"
#include "Player/MAPlayerState.h"
#include "Player/Components/MAPlayerCharacterMovementComponent.h"
#include "Player/Cursor/MACursorSubsystem.h"
#include "Player/Loadout/LoadoutComponent.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutEyeShapePresetData.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Player/Mount/Data/MountData.h"
#include "Player/Revive/MAReviveActor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Shop/MAShopNPC.h"

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

	InventoryComponent = CreateDefaultSubobject<UMAInventoryComponent>("InventoryComponent");
	CurrencyComponent = CreateDefaultSubobject<UMACurrencyComponent>(TEXT("CurrencyComponent"));
	InteractorComponent = CreateDefaultSubobject<UMAInteractorComponent>(TEXT("InteractorComponent"));
	LoadoutComponent = CreateDefaultSubobject<ULoadoutComponent>(TEXT("LoadoutComponent"));

	/** Create SKCs **/
	// Create and Attach Weapon
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("Weapon"));
	WeaponComponent->SetupAttachment(GetMesh(), TEXT("WeaponHandSocket"));
	GetHighlightComponent()->AddTarget(WeaponComponent);

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

	ReviveActorClass = AMAReviveActor::StaticClass();
}

void AMAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (LoadoutComponent)
	{
		LoadoutComponent->InitializeMaterial(GetMesh());
	}
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

UInputAction* AMAPlayerCharacter::GetGameplayAbilityInputAction(FGameplayTag SlotTag) const
{
	if (UInputAction* const* FoundAction = GameplayAbilityInputActions.Find(SlotTag)) return *FoundAction;
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
		
		for (const TPair<FGameplayTag, UInputAction*>& InputActionPair : GameplayAbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &AMAPlayerCharacter::HandleAbilityInputStarted, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &AMAPlayerCharacter::HandleAbilityInputReleased, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Canceled, this, &AMAPlayerCharacter::HandleAbilityInputReleased, InputActionPair.Key);
		}
	}
}

void AMAPlayerCharacter::Server_AddCoin_Implementation(float Amount)
{
	if (CurrencyComponent)
	{
		CurrencyComponent->AddCoin(Amount);
	}
}

void AMAPlayerCharacter::Server_RefreshShopStock_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<AMAShopNPC> It(World); It; ++It)
	{
		It->RefreshStock();
	}
}

void AMAPlayerCharacter::Server_ShopTest_Implementation()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (AMAGameMode* GameMode = World->GetAuthGameMode<AMAGameMode>())
	{
		GameMode->SetMAState(4);
	}

	if (CurrencyComponent)
	{
		CurrencyComponent->AddCoin(99999.f);
	}

	for (TActorIterator<AMAShopNPC> It(World); It; ++It)
	{
		It->SetStockCountsForTest(99);
		It->RefreshStock();
	}
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

	InteractorComponent->Interact(this);
}

void AMAPlayerCharacter::HandleAbilityInputStarted(const FInputActionValue& InputActionValue, FGameplayTag SlotTag)
{
	if (!InputActionValue.Get<bool>()) return;
	if (IsInputBlocked()) return;

	SetAbilityInputHeld(SlotTag, true);
	TryActivateHeldAbilityInput(SlotTag);
}

void AMAPlayerCharacter::HandleAbilityInputReleased(const FInputActionValue& /*InputActionValue*/, FGameplayTag SlotTag)
{
	SetAbilityInputHeld(SlotTag, false);
}

void AMAPlayerCharacter::TickHeldAbilityInputs()
{
	if (IsInputBlocked()) return;

	for (const FGameplayTag& SlotTag : HeldAbilitySlotTags)
	{
		TryActivateHeldAbilityInput(SlotTag);
	}
}

void AMAPlayerCharacter::SetAbilityInputHeld(FGameplayTag SlotTag, bool bHeld)
{
	const int32 SlotInputID = FMASkillSystemStatics::ResolveSlotInputID(SlotTag);
	if (SlotInputID == INDEX_NONE) return;

	if (bHeld)
	{
		HeldAbilitySlotTags.Add(SlotTag);
		GetAbilitySystemComponent()->AbilityLocalInputPressed(SlotInputID);
		return;
	}

	HeldAbilitySlotTags.Remove(SlotTag);
	GetAbilitySystemComponent()->AbilityLocalInputReleased(SlotInputID);
}

void AMAPlayerCharacter::TryActivateHeldAbilityInput(FGameplayTag SlotTag)
{
	if (!SlotTag.IsValid()) return;

	if (UMASkillManagerComponent* SkillManager = GetSkillManagerComponent())
	{
		SkillManager->TryActivateSkill(SlotTag);
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

	if (HasAuthority())
	{
		if (UMASkillManagerComponent* SkillManager = GetSkillManagerComponent())
		{
			UMASkillModule* AttackSkillModule = WeaponDataRow ? WeaponDataRow->AttackSkillModule.LoadSynchronous() : nullptr;
			SkillManager->ReplaceModuleAt(
				FGameplayTag::RequestGameplayTag(TEXT("Skill.Slot.Active.1")),
				0,
				AttackSkillModule);

			UMASkillModule* PassiveSkillModule = WeaponDataRow ? WeaponDataRow->PassiveSkillModule.LoadSynchronous() : nullptr;
			SkillManager->ReplaceModuleAt(
				FMASkillSystemStatics::GetPassiveSlotTag(),
				0,
				PassiveSkillModule);
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
	ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	UMACursorSubsystem* Cursor = LocalPlayer ? LocalPlayer->GetSubsystem<UMACursorSubsystem>() : nullptr;
	return Cursor && Cursor->GetAimDirection(OutDirection);
}

void AMAPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
	SpawnReviveActor();
	if (HasAuthority())
	{
		if (AMAGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMAGameMode>() : nullptr)
		{
			GameMode->CheckGameOver();
		}
	}
	if (LoadoutComponent)
	{
		LoadoutComponent->ApplyMaterialParam(LoadoutComponent->GetMaterialParamValue(), DeadColorSaturationScale);
	}
}

void AMAPlayerCharacter::OnRespawn()
{
	ClearReviveActor();
	SetInputEnabledFromPlayerController(true);

	if (LoadoutComponent)
	{
		LoadoutComponent->ApplyMaterialParam(LoadoutComponent->GetMaterialParamValue());
	}

	if (HasAuthority() && GetAbilitySystemComponent())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		CueParams.Normal = GetActorUpVector();
		CueParams.Instigator = this;
		CueParams.EffectCauser = this;
		GetAbilitySystemComponent()->ExecuteGameplayCue(
			UMAAbilitySystemStatics::GetPlayerRespawnGameplayCueTag(),
			CueParams);
	}
}

void AMAPlayerCharacter::SpawnReviveActor()
{
	if (!HasAuthority() || ActiveReviveActor || !ReviveActorClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	static constexpr float ReviveGroundTraceUp = 200.f;
	static constexpr float ReviveGroundTraceDown = 1000.f;

	FVector SpawnLocation = GetActorLocation();
	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SpawnReviveActor), false, this);
	const FVector TraceStart = SpawnLocation + FVector::UpVector * ReviveGroundTraceUp;
	const FVector TraceEnd = SpawnLocation - FVector::UpVector * ReviveGroundTraceDown;
	if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		SpawnLocation = GroundHit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveReviveActor = World->SpawnActor<AMAReviveActor>(
		ReviveActorClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);
	if (ActiveReviveActor)
	{
		ActiveReviveActor->InitializeReviveTarget(this);
	}
}

void AMAPlayerCharacter::ClearReviveActor()
{
	if (!HasAuthority()) return;

	if (ActiveReviveActor)
	{
		ActiveReviveActor->Destroy();
		ActiveReviveActor = nullptr;
	}
}

void AMAPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
