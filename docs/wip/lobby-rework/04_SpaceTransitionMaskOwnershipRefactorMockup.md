# 04. Space Transition Mask Ownership Refactor Mockup

> Status: Mockup-ready ownership correction
> Branch: codex/mockups
> Engine baseline: Unreal Engine 5.8
> Overrides: 02_Persistent Space Transition Mockup의 MagicCircle / Sphere Mask ownership 부분

## 1. 목적

현재 구현은 각 `AMAMagicCircle`이 자기 `UMASpaceTransitionMaskComponent`를 소유한다.

이 구조에서는 실제 전환이 단순히:

~~~text
Close
-> Closed 유지
-> Open
~~~

이면 되는 상황에서도 Source와 Destination의 서로 다른 Mask를 넘겨 써야 한다.

그 결과 다음과 같은 접착 API가 생겼다.

~~~text
Source.Close()
-> Source.ReleaseClosedState()
-> Destination.SetClosedState()
-> Destination.Open()
~~~

`SetClosedState()`와 `ReleaseClosedState()`는 게임 규칙이 요구하는 상태가 아니라,
Mask ownership이 MagicCircle마다 분리되어 있어서 필요한 연결 작업이다.

이번 리팩터의 목적은 전환 표현의 실제 주인을 바로잡아 정상 경로를 다시 다음으로 단순화하는 것이다.

~~~text
Close(Source Center)
-> Closed 유지
-> player handoff
-> Open(Destination Center)
~~~

## 2. 최종 책임 구조

~~~text
AMAMagicCircle
- Ready / overlap 감지
- 전환 기준 위치 제공
- World <-> Circle 상대 Transform 기준 제공

UMASpaceTransitionSubsystem
- 전체 Space Transition 흐름
- Current / Destination 관리
- Server / Client 진행 동기화
- player handoff
- 하나의 Transition Mask 소유

UMASpaceTransitionMask
- 전환 시각 표현만 담당
- PP Volume / MID lifetime
- Radius close/open animation
- Transition Center
- transition-visible subject 적용/해제
~~~

핵심 원칙:

> MagicCircle은 전환을 실행하지 않는다.
>
> MagicCircle은 전환이 일어나는 기준 위치를 제공한다.
>
> 실제 전환과 Mask lifetime은 SpaceTransitionSubsystem이 소유한다.

## 3. MagicCircle 책임 축소

`AMAMagicCircle`에서 제거할 책임:

~~~text
Mask 생성/소유
CloseTransition
OpenTransition
SetTransitionClosed
ReleaseTransitionPresentation
Mask Phase 관리
PP Volume 관리
Transition Material 관리
~~~

MagicCircle에 남길 책임:

~~~text
ReadyArea / overlap
ReadyPlayers
Ready 판정
Circle transform
WorldToCircleTransform
CircleToWorldTransform
GetActorLocation / GetActorTransform을 통한 transition 기준 위치
~~~

MagicCircle이 TransitionSubsystem에게 Presentation 명령을 전달하는 중간 wrapper가 되지 않는다.

### TransitionVisibilityComponent

MagicCircle Mesh 자체가 Sphere Mask 안에서도 보여야 한다면 기존 `UMASpaceTransitionVisibilityComponent`는 유지할 수 있다.

이 Component는 Transition을 제어하는 기능이 아니라:

~~~text
"이 렌더 대상은 transition mask를 통과해서 보여야 한다"
~~~

라는 선언적 render marker다.

따라서 Mask ownership과는 별개다.

## 4. Mask ownership

Mask는 Source Space의 것도 Destination Space의 것도 아니다.

한 번의 Space Transition 동안 사용되는 하나의 world-local presentation이다.

따라서:

~~~text
UMASpaceTransitionSubsystem
└─ TransitionMask
~~~

구조로 둔다.

정상적으로 동시에 두 개의 Space Transition이 존재하지 않으므로 Mask도 하나면 충분하다.

다음은 만들지 않는다.

~~~text
SourceMask
DestinationMask
Mask registry
Mask pool
per-Space mask state
~~~

## 5. Mask class 형태

현재 `UMASpaceTransitionMaskComponent`는 MagicCircle의 `UActorComponent` default subobject라서 `GetOwner()`를 통해 Center를 얻는다.

새 구조에서는 Mask가 특정 Actor에 속하지 않으므로 이 결합을 제거한다.

목표 개념은 다음과 같다.

~~~text
UMASpaceTransitionMask
= SpaceTransitionSubsystem이 소유하는 presentation helper
~~~

구현 시 ActorComponent 형태를 억지로 유지하기 위해 별도 host Actor를 만들지 않는다.

Mask animation tick을 어떤 Unreal mechanism으로 구동할지는 구현 세부사항이지만 다음 계약은 유지한다.

- Mask lifetime은 TransitionSubsystem에 종속
- Center는 명시적으로 전달
- MagicCircle Actor ownership에 의존하지 않음
- 별도 gameplay Actor를 ownership workaround로 생성하지 않음

## 6. Mask public contract

정상 전환에 필요한 핵심 API는 두 개다.

~~~cpp
bool Close(
    const FVector& Center,
    FSimpleDelegate OnClosed = FSimpleDelegate());

bool Open(
    const FVector& Center,
    FSimpleDelegate OnOpened = FSimpleDelegate());
~~~

의미:

~~~text
Close(SourceCenter)
= Source MagicCircle 위치를 중심으로 Radius를 줄여 화면을 닫는다.

Open(DestinationCenter)
= 완전히 닫힌 상태에서 Center를 Destination MagicCircle 위치로 바꾸고
  Radius를 키워 새 공간을 보여준다.
~~~

정상 경로에 다음 API는 두지 않는다.

~~~text
SetClosedState
ReleaseClosedState
ApplyClosedState
TransferMask
TakeMaskOwnership
~~~

## 7. Closed 상태 유지

`Close()`가 끝나면 Mask는 그대로 `Closed` 상태를 유지한다.

~~~text
Open
-> Closing
-> Closed
~~~~~~~~~~~~~~~
   handoff 동안 유지
~~~~~~~~~~~~~~~
-> Opening
-> Open
~~~

Closed 상태에서:

- PP Volume 유지
- MID 유지
- Radius = ClosedRadius
- transition-visible subject 상태 유지
- Tick은 필요 없으면 중지

Source Mask를 폐기하고 Destination Mask를 다시 만드는 단계는 없다.

## 8. Center handoff

Center 변경은 화면이 완전히 닫힌 동안 수행한다.

~~~text
Source Circle Center
    ↓
Close
    ↓
Closed
    ↓
player handoff
    ↓
Mask Center = Destination Circle Center
    ↓
Open
~~~

Closed 상태에서는 월드가 가려져 있으므로 Source Center에서 Destination Center로 바뀌는 중간 장면을 보여줄 필요가 없다.

별도 `SetClosedState()`가 필요하지 않다.

`Open(DestinationCenter)`가 Opening 시작 직전에 Center를 갱신하면 된다.

## 9. Normal local transition flow

클라이언트 하나의 로컬 Presentation 기준:

~~~text
1. Destination loaded / ready
2. Source MagicCircle 위치 확보
3. TransitionMask.Close(SourceCenter)
4. Close animation 완료
5. Closed 유지
6. player relative transform handoff
7. Destination MagicCircle 위치 확보
8. TransitionMask.Open(DestinationCenter)
9. Open animation 완료
10. Presentation 정리
~~~

Mask 자체를 교체하는 단계는 없다.

## 10. TransitionSubsystem flow 연결

기존 Subsystem의 전체 network / load 흐름은 유지한다.

변경되는 부분은 local presentation 호출 경계다.

### Close 단계

기존 개념:

~~~text
CurrentLevel.MagicCircle.CloseTransition()
~~~

변경:

~~~text
SourceCircle = CurrentLevel의 MagicCircle
SourceCenter = SourceCircle 위치
TransitionMask.Close(SourceCenter, OnClosed)
~~~

### Handoff / Open 단계

기존 개념:

~~~text
Source Circle presentation release
Destination Circle SetClosedState
Destination Circle OpenTransition
~~~

변경:

~~~text
MovePlayersToDestination(...)
DestinationCircle = DestinationLevel의 MagicCircle
DestinationCenter = DestinationCircle 위치
TransitionMask.Open(DestinationCenter, OnOpened)
~~~

즉 Subsystem 입장에서도 정상 경로가:

~~~text
Close
-> Handoff
-> Open
~~~

으로 줄어든다.

## 11. Abort / Deinitialize cleanup

정상 전환 API와 실패 정리 API를 섞지 않는다.

실패 또는 World 종료 시에는 Mask가 현재 어느 Phase에 있든 Presentation을 즉시 정리할 수 있어야 한다.

개념상 내부 cleanup entry point 하나만 둔다.

예:

~~~text
Reset()
~~~

역할:

- active callback 해제
- animation 중지
- visibility marker 해제
- PP Volume 제거
- MID 제거
- Phase = Open

이 함수는 정상 `Close -> Open` 흐름을 구성하는 단계가 아니다.

오직:

- Abort
- Deinitialize
- unrecoverable local failure

정리에 사용한다.

`ReleaseClosedState()`처럼 정상 handoff에 필요한 API로 사용하지 않는다.

## 12. Presentation lifetime

### Close 시작

필요하면 생성:

~~~text
PP Volume
MID
~~~

그리고:

~~~text
Center = SourceCenter
transition-visible subjects enable
Radius animation start
~~~

### Closed

Presentation 그대로 유지.

새 PP Volume을 생성하지 않는다.

### Open 완료

~~~text
transition-visible subjects disable
PP Volume destroy
MID release
Phase = Open
~~~

한 Transition 안에서 Presentation 생성/파괴는 기본적으로 한 쌍이다.

## 13. Transition-visible subjects

기존 world-wide `UMASpaceTransitionVisibilityComponent` 수집 정책은 유지한다.

현재 요구에서는 같은 UWorld에서 transition-visible 대상으로 선언된 Component를 활성화하면 충분하다.

이번 ownership 리팩터를 이유로 다음을 추가하지 않는다.

~~~text
ULevel filter
Space별 visible subject registry
Source/Destination별 subject list
별도 render ownership graph
~~~

흐름:

~~~text
Close start
-> SetVisibleSubjectsEnabled(true)

Open finish / Reset
-> SetVisibleSubjectsEnabled(false)
~~~

## 14. Phase

Mask 자체의 최소 Phase는 유지 가능하다.

~~~cpp
enum class EPhase : uint8
{
    Open,
    Closing,
    Closed,
    Opening
};
~~~

이 Phase는 Presentation 내부 상태다.

`UMASpaceTransitionSubsystem::EPhase`와 의미가 다르다.

~~~text
Subsystem Phase
= network / streaming / handoff orchestration 상태

Mask Phase
= 시각적 radius animation 상태
~~~

둘을 하나의 enum으로 합치지 않는다.

## 15. 제거 대상 요약

### AMAMagicCircle

제거:

~~~text
UMASpaceTransitionMaskComponent default subobject
Mask 관련 UPROPERTY
CloseTransition()
OpenTransition()
SetTransitionClosed()
ReleaseTransitionPresentation()
~~~

### Mask

제거:

~~~text
GetOwner()를 Center source로 사용하는 결합
SetClosedState()
ReleaseClosedState()
Source/Destination Mask handoff 개념
~~~

### SpaceTransitionSubsystem

제거되는 흐름:

~~~text
Source presentation release
Destination forced closed presentation 생성
~~~

추가/유지:

~~~text
하나의 TransitionMask 소유
Close(SourceCenter)
Open(DestinationCenter)
Abort/Deinitialize 시 Reset
~~~

## 16. 변경하지 않는 범위

이번 리팩터는 Mask ownership만 바로잡는다.

다음은 그대로 유지한다.

- Persistent same-UWorld streaming 구조
- `AMALevelRoot`
- `UMAStreamingLevelLoader`
- Destination prepare first
- Server / Client readiness 동기화
- `FMASpaceTransitionRequest`
- `DestinationInstanceIdentity`
- `GenerationSeed`
- PlayerController RPC network boundary
- relative player transform handoff
- transition stencil high bit `0x80`
- highlight stencil low 7 bits `0x7F`
- `UMASpaceTransitionVisibilityComponent`
- Source unload after Destination activation/open

이번 작업을 이유로 위 구조를 다시 설계하지 않는다.

## 17. 02 Mockup과의 관계

02에서 다음 책임은 폐기한다.

~~~text
MagicCircle
- Sphere Mask / 전환 연출
- Sphere Mask close/open presentation
~~~

04 이후의 정확한 계약:

~~~text
MagicCircle
- Ready / overlap
- transition 기준 위치 / transform

SpaceTransitionSubsystem
- transition orchestration
- one TransitionMask ownership

TransitionMask
- close / closed hold / open presentation
~~~

02의 Persistent UWorld, Destination prepare, player handoff, readiness 등의 큰 구조는 그대로 유효하다.

## 18. Acceptance Criteria

### Ownership

- MagicCircle이 Mask를 소유하지 않는다.
- MagicCircle이 Close/Open Presentation wrapper를 제공하지 않는다.
- TransitionSubsystem이 로컬 Transition Mask 하나를 소유한다.
- Mask Center는 명시적으로 Source/Destination 위치를 전달받는다.

### Normal Flow

정상 transition presentation이 반드시 다음으로 읽혀야 한다.

~~~text
Close(SourceCenter)
-> Closed 유지
-> player handoff
-> Open(DestinationCenter)
~~~

정상 흐름에서 다음 호출이 없어야 한다.

~~~text
SetClosedState
ReleaseClosedState
SourceMask -> DestinationMask transfer
~~~

### Presentation

- Close 시작 시 Presentation 생성
- Closed 동안 같은 Presentation 유지
- Open 완료 시 Presentation 제거
- transition-visible subject는 Close 시작부터 Open 완료까지 유지

### Failure Cleanup

- Abort/Deinitialize에서 한 번의 Reset으로 Presentation을 즉시 정리할 수 있다.
- Reset은 정상 handoff 단계로 사용하지 않는다.

### Scope

- Mask ownership 변경을 이유로 network/request/streaming architecture를 다시 확장하지 않는다.
- generic presentation manager, registry, pool을 추가하지 않는다.

## 19. 구현 순서

### A. Mask ownership 분리

1. Mask가 MagicCircle owner 위치에 의존하지 않도록 Center를 명시적 입력으로 변경
2. Mask lifetime을 TransitionSubsystem 쪽으로 이동
3. 정상 API를 `Close(Center)` / `Open(Center)`로 정리
4. Abort/Deinitialize용 즉시 cleanup 하나 유지

### B. MagicCircle 정리

1. Mask default subobject 제거
2. Mask wrapper API 제거
3. Ready / overlap / transform 기능만 유지
4. 필요한 declarative TransitionVisibility marker는 유지

### C. Subsystem 연결

1. Source Circle 위치로 Close
2. Close 완료 후 기존 network handoff 진행
3. player relative transform 이동
4. Destination Circle 위치로 Open
5. Open 완료 후 기존 promotion/unload 흐름 진행

### D. 기존 접착 로직 제거

1. `SetClosedState()` 호출 제거
2. `ReleaseClosedState()` 호출 제거
3. Source/Destination Presentation transfer 관련 분기 제거
4. 더 이상 사용되지 않는 Mask wrapper / field 정리

## 20. 핵심 요약

~~~text
MagicCircle
= 감지 + 기준 위치

SpaceTransitionSubsystem
= 전환의 주인
= Mask 하나 소유

TransitionMask
= Close(SourceCenter)
= Closed 유지
= Open(DestinationCenter)
~~~

이 구조의 목표는 새로운 abstraction을 추가하는 것이 아니다.

기존에 두 개의 MagicCircle Mask 사이를 연결하기 위해 생긴 불필요한 상태 조작을 제거하고,
실제 게임 규칙인 `Close -> Closed -> Open`을 코드 구조에도 그대로 반영하는 것이다.
