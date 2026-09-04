# 05. Camera Architecture Refactor Mockup

> Status: Final implementation contract
> Target branch: feature/psw/level-system-rework
> Reviewed local-change base HEAD: 3cdd66eea50bd481285f27c1f9976b1e7adc2062
> Mockup branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8
> Supersedes: 04_CurrentImplementationFeedbackMockup.md section 14 for camera code

## 1. 목적

현재 `UMAPlayerCameraDirectorComponent`는 여러 곳에 흩어진 카메라 기능을 하나의 재사용 가능한 진입점으로 모으기 위해 추가됐다.

현재는 다음 책임이 한 객체에 함께 들어 있다.

- ViewTarget 전환
- Player Camera Rig 보간
- Legacy Lobby CameraActor Transform/FOV 보간
- Fade
- Presentation View
- Presentation Fill Light
- Camera Occlusion Cutout 연결
- Client RPC
- Tick/Timer와 각 기능의 지속 상태

이번 리팩터링의 목적은 카메라 기능을 더 잘게 쪼개는 것이 아니다.

목표는 다음과 같다.

> 특정 카메라 자체의 지속 상태는 그 카메라가 소유한다.
>
> 특정 객체 하나에 속하지 않는 공용 기술만 stateless static API로 둔다.
>
> 네트워크 경계는 PlayerController가 소유한다.
>
> Shop, Spectate, Loadout, Ready 같은 실제 사용처가 자기 연출 순서와 로컬 효과 수명을 소유한다.
>
> 현재 소비자가 없는 미래용 Camera API는 만들지 않는다.
>
> CameraDirector라는 중앙 만능 객체는 제거한다.

## 2. 최종 큰 구조

~~~text
Player/Camera/
├─ MACameraComponent.h/.cpp
├─ MACameraLibrary.h/.cpp
├─ MACameraTypes.h
└─ MACameraOcclusionCutoutComponent.h/.cpp

Player/
└─ MAPlayerControllerBase

실제 사용처
├─ Ready
├─ Shop
├─ Spectate
├─ Hub Loadout
└─ Legacy Lobby
~~~

책임은 다음처럼 고정한다.

~~~text
UMACameraComponent
= Player Camera Rig의 시간 기반 보간 상태를 소유한다.

FMACameraLibrary
= 상태를 보관하지 않는 ViewTarget/Fade/공용 Presentation Light 기술을 제공한다.

AMAPlayerControllerBase
= 서버/클라이언트 카메라 요청의 RPC 경계와 network fade의 최소 Timer 상태를 소유한다.

UMACameraOcclusionCutoutComponent
= 기존처럼 로컬 Cutout 상태와 계산을 소유한다.

Ready / Shop / Spectate / Hub Loadout / Legacy Lobby
= 어떤 카메라 기능을 언제 어떤 순서로 사용할지 직접 소유한다.
~~~

`UMAPlayerCameraDirectorComponent`는 제거한다.

## 3. UMACameraComponent

새 `UMACameraComponent`는 `UCameraComponent`를 상속한다.

~~~cpp
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class P_MA_API UMACameraComponent : public UCameraComponent
~~~

첫 구현에서 이 컴포넌트가 소유하는 시간 기반 기능은 실제 소비자가 있는 Camera Rig 보간 하나다.

### 3.1 Rig transition

기존 `TransitionPawnCamera()`는 Player 전용 정책이 아니다.

실제 기능은 다음이다.

~~~text
SpringArm
- TargetArmLength
- Pitch
- TargetOffset

Camera
- FOV

를 일정 시간 동안 목표 설정으로 보간
~~~

따라서 이름에서 Pawn을 제거한다.

개념 public API:

~~~cpp
void SetRig(
    USpringArmComponent& SpringArm,
    const FMACameraRigSettings& Settings);

void TransitionRig(
    USpringArmComponent& SpringArm,
    const FMACameraRigSettings& Settings);

void StopTransition();
~~~

`SpringArm`은 영구 멤버로 캐시하지 않는다.

`TransitionRig()` 호출 시 전달받고 transition 중에만 weak reference로 유지한다.

`UMACameraComponent`는 PlayerController, Pawn 탐색, Ready 상태를 모른다.

### 3.2 Tick

Rig 보간에 필요한 Tick은 별도 Director가 아니라 이 CameraComponent의 `TickComponent()`를 사용한다.

기본:

~~~text
bCanEverTick = true
bStartWithTickEnabled = false
~~~

보간 시작:

~~~text
TransitionRig()
-> 현재 Rig/FOV 값을 start로 캡처
-> Tick 활성화
~~~

완료 또는 취소:

~~~text
Finish / StopTransition / SetRig
-> Tick 비활성화
~~~

항상 Tick하지 않는다.

### 3.3 Interruption contract

보간 중 새 Rig transition이 들어오면 현재 값에서 새 목표로 이어간다.

~~~text
A -> B 보간 중 현재 값이 C
새 목표 D 요청

C -> D로 새 보간 시작

B로 snap하지 않음
A로 되돌리지 않음
~~~

새 `TransitionRig()`는 현재 값을 새 start로 캡처하고 elapsed를 0으로 다시 시작한다.

`SetRig()`는 진행 중 transition을 취소하고 목표 값을 즉시 적용한다.

### 3.4 보간 계산

현재 Rig 계약은 그대로 보존한다.

~~~text
- ArmLength
- Pitch
- TargetOffset
- FOV
- Duration
- EaseExponent
- PulseFOVDelta
~~~

현재 필요한 계산을 위한 private 함수만 둔다.

예:

~~~text
UpdateRigTransition()
ApplyRigStep()
FinishRigTransition()
~~~

generic Camera Transition base class, strategy hierarchy, interpolation framework를 만들지 않는다.

### 3.5 View transition은 이번에 만들지 않는다

기존 Director의 `InterpExternalCameraView()` / `TeleportExternalCameraView()`는 Legacy Lobby CameraActor를 위한 기능이었다.

현재 Legacy Lobby camera는 native CameraActor이며, 이를 `UMACameraComponent`로 교체하려면 별도 CameraActor hierarchy 또는 큰 asset 수정이 필요하다.

현재 active 구조에서 `UMACameraComponent::TransitionView()`의 실제 소비자는 없다.

따라서 이번 구현에서는 다음을 만들지 않는다.

~~~text
UMACameraComponent::SetView()
UMACameraComponent::TransitionView()
FMACameraViewSettings
FMACameraViewInterpSettings
View transition state
View transition Tick
~~~

Legacy Lobby CameraActor Transform/FOV 보간은 `ALobbyPlayerController` 내부 legacy-local 구현으로 유지한다.

미래에 실제 `UMACameraComponent` View 보간 사용처가 생기면 그때 구체적인 요구를 보고 추가한다.

### 3.6 UMACameraComponent가 소유하지 않는 것

- PlayerController
- Pawn 탐색
- Possess/AcknowledgePossession 감시
- ViewTarget 선택 정책
- Fade
- RPC
- Cutout
- Presentation Fill Light lifetime
- Shop/Loadout/Spectate/Ready 의미
- Legacy Lobby CameraActor
- 어느 카메라를 사용할지 결정하는 정책

## 4. FMACameraLibrary

`FMACameraLibrary`는 UObject/ActorComponent가 아닌 plain C++ static utility class로 둔다.

Blueprint 노출 요구가 실제로 생기기 전에는 `UBlueprintFunctionLibrary`로 만들지 않는다.

개념 public API:

~~~cpp
class FMACameraLibrary final
{
public:
    static void SwitchViewTarget(
        APlayerController& PlayerController,
        AActor& ViewTarget,
        float BlendTime = 0.f);

    static void SwitchToPawn(
        APlayerController& PlayerController,
        float BlendTime = 0.f);

    static void FadeOut(
        APlayerController& PlayerController,
        float Duration);

    static void FadeIn(
        APlayerController& PlayerController,
        float Duration);

    static void StopFade(
        APlayerController& PlayerController);

    static USpotLightComponent* CreatePresentationFillLight(
        AActor& Owner,
        USceneComponent& AttachParent,
        const FMACameraPresentationSettings& Settings);

    static void DestroyPresentationFillLight(
        USpotLightComponent*& FillLight);
};
~~~

정확한 함수 시그니처는 구현 중 현재 호출처에 맞춰 최소화할 수 있지만 책임 경계는 바꾸지 않는다.

Library는 반환한 Fill Light를 보관하지 않는다.

## 5. ViewTarget

`SwitchViewTarget()`가 실제 public 진입점이다.

별도:

~~~text
SwitchViewTarget()
-> BlendToViewTarget()
~~~

같은 한 겹 wrapper를 만들지 않는다.

함수 내부에서 직접:

- local controller 확인
- BlendTime clamp
- 즉시 SetViewTarget 또는 SetViewTargetWithBlend

만 닫는다.

현재 소비자가 없는 Blend 완료 delegate/handle은 만들지 않는다.

`SwitchToPawn()`은 Pawn을 얻은 뒤 `SwitchViewTarget()`을 재사용한다.

`RefreshPawnCamera()`는 삭제한다.

### Cutout은 ViewTarget Library 책임이 아니다

ViewTarget과 Reveal Target은 항상 같은 객체가 아니다.

~~~text
Spectate
ViewTarget = SpectateTarget
RevealTarget = SpectateTarget

Pawn 복귀
ViewTarget = Pawn
RevealTarget = Pawn

Presentation
ViewTarget = Presentation Camera
RevealTarget = Preview Subject
~~~

따라서 `FMACameraLibrary::SwitchViewTarget()`은 Cutout target을 자동으로 변경하지 않는다.

Cutout 연결은 실제 의미를 아는 사용처가 결정한다.

## 6. Fade

Fade는 CameraComponent 자체의 상태가 아니라 Player 화면 효과다.

실제 렌더 Fade는 기존처럼 `APlayerCameraManager::StartCameraFade()`를 사용한다.

Library는 Fade sequence, Timer, callback 상태를 소유하지 않는다.

~~~text
FadeOut()
= StartCameraFade(0 -> 1)

FadeIn()
= StartCameraFade(1 -> 0)

StopFade()
= StopCameraFade()
~~~

`FadeOut -> 기다림 -> 다른 작업 -> FadeIn` 같은 순서는 실제 사용처가 소유한다.

필요한 `FTimerHandle`도 실제 사용처가 가진다.

~~~text
Shop
- ShopCameraFadeTimerHandle

Legacy Lobby
- LobbyCameraFadeTimerHandle

PlayerController network fade
- CameraFadeTimerHandle
~~~

별도 Fade Manager, Fade Component, static global TMap을 만들지 않는다.

## 7. Presentation Fill Light

현재 Presentation Fill Light는 Hub Loadout만이 아니라 Shop에서도 실제로 사용한다.

따라서 Fill Light의 수명은 각 사용처가 독립적으로 소유한다.

~~~text
Shop
└─ PresentationFillLight

Hub Loadout
└─ PresentationFillLight
~~~

두 사용처가 같은 생성/설정/삭제 기술을 사용하므로 그 기술만 `FMACameraLibrary`에 stateless helper로 둔다.

~~~text
사용처
-> CreatePresentationFillLight(...)
-> 반환된 USpotLightComponent를 자기 멤버로 보관
-> 필요한 동안 사용
-> DestroyPresentationFillLight(...)
~~~

Library는 현재 Fill Light pointer나 presentation 상태를 보관하지 않는다.

`FMACameraPresentationSettings`는 Shop과 Hub Loadout이라는 두 실제 소비자가 있으므로 공용 데이터 타입으로 유지한다.

단, 설정 값의 owner는 CameraDirector가 아니라 각 실제 사용처다.

### Presentation Settings migration

기존 Controller Blueprint / CameraDirector에 저장된 `PresentationSettings` 값이 있을 수 있다.

Director를 삭제하기 전에 실제 값을 확인하고 Shop과 Hub Loadout의 새 설정 위치로 이전한다.

~~~text
기존 Controller BP PresentationSettings 확인
-> Shop PresentationSettings에 값 이전
-> Hub Loadout PresentationSettings에 값 이전
-> Editor에서 동일 동작 확인
-> 그 뒤 CameraDirector 제거
~~~

Owner가 달라지는 데이터이므로 단순 PropertyRedirect만으로 이전됐다고 가정하지 않는다.

기존 튜닝값 확인 없이 Director component를 먼저 삭제하지 않는다.

## 8. Camera Occlusion Cutout 범위

`UMACameraOcclusionCutoutComponent`는 이번 카메라 구조 정리의 내부 리팩터링 대상이 아니다.

이번 범위:

~~~text
내부 계산 수정 X
Material parameter 정책 수정 X
Stencil/렌더 방식 수정 X
컴포넌트 구조 수정 X

CameraDirector 의존성 제거 O
호출 위치 재배치 O
기존 동작 유지 O
~~~

RevealTarget/ClearTarget 호출은 실제 의미를 아는 위치로 이동한다.

- possession: PlayerController
- spectate: Spectate
- presentation: Shop / Hub Loadout
- pawn 복귀: 해당 복귀를 요청한 사용처

CameraDirector 삭제 때문에 Possess 시 Cutout 초기화가 사라져서는 안 된다.

현재 `RefreshPawnCamera()`가 하던 CameraBoom/Camera 캐시는 제거하지만 Cutout 연결은 PlayerController possession 경계로 옮긴다.

~~~text
OnPossess / AcknowledgePossession
-> local controller 확인
-> Cutout ClearTarget()
-> 현재 Pawn RevealTarget()
~~~

필요하면 이 연결만 닫는 private helper를 PlayerController에 둔다.

## 9. AMAPlayerControllerBase

PlayerController는 더 이상 CameraDirectorComponent를 소유하지 않는다.

제거:

~~~text
CameraDirectorComponent
GetCameraDirector()
OnPossess()의 RefreshPawnCamera()
AcknowledgePossession()의 RefreshPawnCamera()
~~~

기존 `UMACameraOcclusionCutoutComponent`는 유지한다.

### Camera RPC

서버가 특정 owning client 화면에 Fade를 요청해야 하는 실제 경계는 PlayerController다.

현재 Director의:

~~~text
RequestFade()
ClientRequestFade()
~~~

중 network boundary만 PlayerController로 이동한다.

개념:

~~~cpp
void RequestCameraFade(const FMACameraFadeSettings& Settings);

UFUNCTION(Client, Reliable)
void ClientPlayCameraFade(const FMACameraFadeSettings& Settings);
~~~

동작:

~~~text
local controller
-> RequestCameraFade()
-> FadeOut
-> FadeOutSeconds timer
-> FadeIn

server owning remote player
-> ClientPlayCameraFade()
-> 같은 local sequence 실행
~~~

이 network-requested full fade의 최소 상태는 PlayerController가 소유한다.

~~~text
FTimerHandle CameraFadeTimerHandle
~~~

새 network fade 요청이 들어오면 기존 timer를 clear하고 `FMACameraLibrary::StopFade()` 후 새 sequence를 시작한다.

PlayerController는 실제 렌더 fade 계산을 구현하지 않고 Library primitive만 호출한다.

## 10. MACameraTypes

현재 데이터 타입은 실제 소비자를 기준으로 정리한다.

### Rig settings rename

~~~text
FMAPlayerCameraRigSettings
-> FMACameraRigSettings
~~~

이 타입은 `USTRUCT(BlueprintType)`이며 Blueprint asset에 저장되어 있을 수 있다.

따라서 이름 변경 시 CoreRedirect를 반드시 추가한다.

개념:

~~~ini
+StructRedirects=(OldName="/Script/P_MA.MAPlayerCameraRigSettings",NewName="/Script/P_MA.MACameraRigSettings")
~~~

정확한 redirect 표기는 프로젝트의 기존 CoreRedirect 형식과 맞춘다.

Redirect 추가 후:

~~~text
C++ Build
-> Editor load
-> 관련 BP compile
-> 값 유지 확인
-> 필요 시 resave
~~~

를 수행한다.

### 유지

~~~text
FMACameraRigSettings
FMACameraFadeSettings
FMACameraPresentationSettings
~~~

### 삭제

Director 제거 후 다른 소비자가 없는 다음 타입은 제거한다.

~~~text
FMACameraViewTarget
FMACameraInterpMoveSettings
~~~

이번 구현에서는 대체용으로 다음 타입을 새로 만들지 않는다.

~~~text
FMACameraViewSettings
FMACameraViewInterpSettings
~~~

Legacy Lobby는 자신의 기존 `FLoadoutCameraViewSettings`와 local camera state를 사용한다.

## 11. 기존 호출처 이전 기준

### ReadyState

기존:

~~~text
GetCameraDirector()
-> RefreshPawnCamera()
-> TransitionPawnCamera(Settings)
~~~

목표:

~~~text
PlayerCharacter.GetPlayerCamera()
-> UMACameraComponent
-> TransitionRig(PlayerCharacter.GetCameraBoom(), Settings)
~~~

ReadyState는 Camera interpolation 내부 구현을 알지 않는다.

### Legacy Lobby Camera

기존 Director의:

~~~text
InterpExternalCameraView()
TeleportExternalCameraView()
~~~

는 `ALobbyPlayerController`의 legacy-local CameraActor Transform/FOV 처리로 되돌린다.

~~~text
ALobbyPlayerController
├─ LobbyCameraActor
├─ LobbyCameraComponent
├─ target Transform/FOV
├─ interpolation Tick/state
└─ Lobby 전용 Fade sequence
~~~

기존 동작은 유지하되 새 CameraActor subclass, 새 Camera Manager, 새 generic View transition abstraction을 만들지 않는다.

Legacy Lobby 하나 때문에 active camera architecture를 복잡하게 만들지 않는다.

### Shop

Shop은 자신의 전체 연출 순서를 소유한다.

~~~text
Open
-> FadeOut
-> Timer
-> SwitchViewTarget
-> Cutout RevealTarget
-> Presentation Fill Light 생성
-> FadeIn

Close
-> FadeOut
-> Timer
-> Presentation Fill Light 제거
-> SwitchToPawn
-> Cutout Pawn 복구
-> FadeIn
~~~

Shop이 자신의 `FTimerHandle`, `PresentationFillLight`, `PresentationSettings`를 소유한다.

빠른 open/close에서 이전 sequence가 남지 않도록 새 sequence 시작 전 자신의 Timer를 정리한다.

CameraLibrary는 이 순서를 알지 않는다.

### Spectate

기존:

~~~text
GetCameraDirector()
-> SwitchToViewTarget(SpectateTarget)
~~~

목표:

~~~text
FMACameraLibrary::SwitchViewTarget(
    PlayerController,
    SpectateTarget,
    BlendTime)

CameraOcclusionCutout
-> RevealTarget(SpectateTarget)
~~~

Pawn 복귀도 Spectate가 직접:

~~~text
SwitchToPawn()
-> Cutout Pawn 복구
~~~

를 호출한다.

### SplineSectorManager network fade

현재 `CompleteLoopReady()`의 `RequestFade()` 호출은 CameraDirector RPC를 사용한다.

목표:

~~~text
Server SplineSectorManager
-> AMAPlayerControllerBase::RequestCameraFade(Settings)
-> owning client RPC
-> PlayerController local fade sequence
~~~

SplineSectorManager는 fade 구현이나 timer를 알지 않는다.

### Hub Loadout Presentation

Hub Loadout은 자신의 Presentation 연출 순서와 상태를 직접 소유한다.

~~~text
Enter
-> SwitchViewTarget
-> Cutout RevealTarget(PreviewSubject)
-> Presentation Fill Light 생성

Exit
-> Presentation Fill Light 제거
-> SwitchToPawn
-> Cutout Pawn 복구
~~~

Hub Loadout이 자신의 `PresentationFillLight`와 `PresentationSettings`를 소유한다.

Shop과 공통인 Fill Light 생성/삭제 기술만 CameraLibrary를 사용한다.

## 12. Player Camera 교체

`AMAPlayerCharacter`의 기본 Camera subobject는 `UCameraComponent`에서 `UMACameraComponent`로 변경한다.

기존 subobject의 의미와 이름은 반드시 유지한다.

~~~text
기존
UCameraComponent* Cam

목표
UMACameraComponent* Cam
~~~

현재 constructor의 subobject name `"Cam"`은 변경하지 않는다.

~~~cpp
Cam = CreateDefaultSubobject<UMACameraComponent>("Cam");
~~~

CameraBoom의 기존 subobject name도 변경하지 않는다.

불필요한 Player 전용 Camera wrapper를 별도로 만들지 않는다.

Blueprint/asset compatibility 검증 순서:

~~~text
C++ Build
-> Editor load
-> 관련 Player Blueprint compile
-> camera/socket/property 연결 확인
-> 필요 시 resave
~~~

단순 class 교체를 이유로 기존 Blueprint 자산을 새 이름의 컴포넌트로 재작성하지 않는다.

## 13. 삭제 대상

파일:

~~~text
MAPlayerCameraDirectorComponent.h
MAPlayerCameraDirectorComponent.cpp
~~~

Director API:

~~~text
GetCameraDirector()
RefreshPawnCamera()
BlendToViewTarget()
EnterPresentationView()
ExitPresentationView()
TransitionPawnCamera()
InterpExternalCameraView()
TeleportExternalCameraView()
RequestFade()
ClientRequestFade()
PlayFade()
FadeOut()
FadeIn()
~~~

Director state:

~~~text
CameraBoom
Camera

RigTransition*
ExternalCamera*
FadeTimerHandle
PendingFadeFinishedAction
bStopCameraFadeOnFinish
PresentationFillLight
PresentationSettings
~~~

타입:

~~~text
FMACameraViewTarget
FMACameraInterpMoveSettings
~~~

대체 타입을 만들지 않는다.

기능은 다음처럼 실제 owner로 이동한다.

~~~text
RigTransition*
-> UMACameraComponent

Legacy External Camera interpolation
-> ALobbyPlayerController

ViewTarget primitive
-> FMACameraLibrary

Fade primitive
-> FMACameraLibrary

Network fade sequence/timer
-> AMAPlayerControllerBase

Shop fade/presentation state
-> Shop

Hub presentation state
-> Hub Loadout

Spectate ViewTarget/Cutout sequence
-> Spectate

Possess Cutout restore
-> AMAPlayerControllerBase
~~~

## 14. 만들지 않을 것

- CameraDirector 대체 Manager
- CameraService Subsystem
- CameraRegistry
- generic Camera transition object hierarchy
- Camera strategy/interface hierarchy
- UMACameraComponent View transition without a current consumer
- FMACameraViewSettings
- FMACameraViewInterpSettings
- static TMap<PlayerController, CameraState>
- generic Presentation Manager
- generic Presentation Context
- generic Camera Command queue
- 모든 카메라 기능을 반드시 하나의 API로 통과시키는 규칙
- Legacy Lobby만을 위한 새 CameraActor hierarchy

카메라 자체 지속 상태는 CameraComponent가 소유한다.

공통 단발 기술만 Library를 사용한다.

연출 순서는 실제 사용처가 소유한다.

## 15. 구현 순서

1. 기존 Controller BP의 PresentationSettings 실제 값 확인
2. `FMACameraRigSettings` rename + StructRedirect 추가
3. `UMACameraComponent`에 Rig transition만 구현
4. Player Camera를 `UMACameraComponent`로 교체하고 subobject name 유지
5. Ready Camera Rig 호출 이전
6. Legacy Lobby External Camera interpolation을 `ALobbyPlayerController` local 구현으로 이전
7. `FMACameraLibrary`의 ViewTarget/Fade primitive 구현
8. 공용 Presentation Fill Light create/destroy helper 구현
9. Shop에 Fade Timer, Fill Light lifetime, PresentationSettings 이전
10. Hub Loadout에 Fill Light lifetime, PresentationSettings 이전
11. Shop/Hub의 기존 BP Presentation tuning 값 복구 확인
12. Spectate ViewTarget/Cutout 연결 이전
13. SplineSectorManager network fade를 PlayerController RPC로 이전
14. Possess/AcknowledgePossession Cutout 초기화 연결 이전
15. CameraDirectorComponent와 getter/lifecycle 연결 제거
16. `FMACameraViewTarget`, `FMACameraInterpMoveSettings`의 남은 소비자 확인 후 삭제
17. `MAPlayerCameraDirectorComponent` 파일 삭제
18. CameraDirector 전체 참조 검색 후 0건 확인
19. CoreRedirect / Player Camera subobject / Blueprint compatibility 검증
20. Standalone 및 Listen Server 기능 회귀 검증
21. 카메라 리팩터링만 별도 선커밋

Director 삭제는 BP Presentation tuning 값 이전과 호출처 이전이 끝난 뒤에 수행한다.

## 16. Acceptance Criteria

### 구조

- `UMAPlayerCameraDirectorComponent`가 없다.
- PlayerController가 CameraDirectorComponent를 소유하지 않는다.
- Player Camera는 `UMACameraComponent`다.
- `UMACameraComponent`는 현재 실제 소비자가 있는 Rig transition만 소유한다.
- `UMACameraComponent::SetView()` / `TransitionView()`가 없다.
- ViewTarget과 Fade primitive는 stateless Library API다.
- Library가 persistent Camera state, Timer, callback을 소유하지 않는다.
- Camera network RPC와 network fade의 최소 Timer state는 PlayerController 경계에 있다.
- Shop과 Hub Loadout이 각각 자신의 Presentation Fill Light lifetime을 소유한다.
- 공통 Fill Light 생성/삭제 기술만 stateless helper로 재사용한다.
- `UMACameraOcclusionCutoutComponent` 내부 구현은 변경하지 않는다.

### 최소성

- Camera Manager/Subsystem을 새로 만들지 않는다.
- Camera transition 상태를 global/static map에 보관하지 않는다.
- CameraComponent가 Pawn/Shop/Spectate/Ready 정책을 알지 않는다.
- 현재 소비자가 없는 View transition API/type을 만들지 않는다.
- Legacy Lobby 하나 때문에 새 CameraActor hierarchy를 만들지 않는다.
- 현재 사용처가 없는 callback/framework를 추가하지 않는다.
- Presentation Manager/Context/Handle을 만들지 않는다.

### Asset / serialization

- Player Camera subobject name `"Cam"`이 유지된다.
- CameraBoom의 기존 subobject name이 유지된다.
- `FMAPlayerCameraRigSettings -> FMACameraRigSettings` StructRedirect가 존재한다.
- 관련 Blueprint에서 Rig settings 값이 유지된다.
- 기존 Controller BP의 PresentationSettings tuning 값을 Shop과 Hub Loadout으로 이전한 뒤 Director를 제거한다.
- Editor load / BP compile 후 camera/property 연결이 끊기지 않는다.

### 동작 유지

- Ready Camera Rig 전환이 기존처럼 부드럽게 동작한다.
- Rig 보간 중 새 목표가 들어오면 현재 값에서 새 목표로 자연스럽게 이어진다.
- Legacy Lobby Camera Transform/FOV 보간과 즉시 이동이 유지된다.
- Shop Fade -> Camera 전환 -> Presentation -> Fade In 흐름이 유지된다.
- Shop Presentation Fill Light가 기존처럼 유지되고 Close 시 제거된다.
- Spectate ViewTarget 전환과 Pawn 복귀가 유지된다.
- Hub Loadout Presentation의 ViewTarget, Cutout, Fill Light가 유지된다.
- local-only camera presentation이 서버/다른 client 화면에 영향을 주지 않는다.
- existing Camera Occlusion Cutout 동작이 유지된다.
- Possess/AcknowledgePossession 이후 Pawn Cutout target이 기존처럼 복구된다.
- SplineSectorManager의 remote player fade가 기존처럼 동작한다.

## 17. 구현 후 읽혀야 하는 형태

~~~text
Ready
  |
  v
UMACameraComponent
- SetRig
- TransitionRig
- StopTransition
- own Rig Tick/state


Shop / Hub / Spectate
  |
  | ViewTarget / Fade / Presentation Light primitive
  v
FMACameraLibrary
- static
- stateless


Server
  |
  | owning client 화면 Fade
  v
AMAPlayerControllerBase RPC
- network fade Timer
  |
  v
FMACameraLibrary


Legacy Lobby
- CameraActor Transform/FOV interpolation local 유지


CameraOcclusionCutoutComponent
- 내부 구현 유지
- 각 사용처가 target만 정확히 연결
~~~

최종 목표는 카메라 기능을 한 군데에 다시 몰아넣는 것이 아니다.

현재 실제로 존재하는 책임만 정확한 owner에게 돌려놓고, 둘 이상의 실제 사용처가 공유하는 작은 기술만 공용화한다.
