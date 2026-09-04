# 04. Current Level System Implementation Feedback Mockup

> Status: Implementation feedback contract
> Target branch: feature/psw/level-system-rework
> Reviewed local-change base HEAD: 3cdd66eea50bd481285f27c1f9976b1e7adc2062
> Mockup branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8

## 1. 목적

현재 Persistent Space Transition 구현은 기능적으로 동작하는 버전이다.

이번 수정의 목적은 기능을 더 추가하는 것이 아니라, 현재 동작을 유지한 채 첫 구현에 붙은 불필요한 책임과 선행 추상화를 덜어내는 것이다.

핵심 목표:

> 작동하기 위한 최소한의 코드만 남긴다.
>
> 실제 책임이 확인된 객체만 둔다.
>
> 아직 발생하지 않은 문제를 위한 Validator, Context, Registry, Readiness Aggregator, Lifecycle Framework를 미리 만들지 않는다.
>
> 기능이 추가되어도 현재 public 진입점과 큰 책임 경계를 뒤집지 않는 최소 기반만 남긴다.

이 문서는 02, 03의 기존 설계 계약 중 현재 구현을 보고 수정이 필요하다고 판단한 부분을 기록하는 피드백 목업이다.

## 2. 현재 구현에서 확인된 구조

현재 구현의 중심은 다음과 같다.

~~~text
UMASpaceTransitionSubsystem
- RequestTransition()
- RequestId / ActiveRequest
- Destination streaming 직접 수행
- streamed AMASpace 등록 수신
- AMASpace::Prepare() 호출
- client DestinationReady / Closed / Opened 수집
- Close / Handoff / Open
- Source unload 직접 수행

AMASpace
- TransitionAnchor
- Loaded / Prepared / Active / Retiring
- BeginPlay / EndPlay에서 TransitionSubsystem 등록
- Prepare / Activate / Retire

AMAPlayerControllerBase
- Server / Client RPC bridge

AMAMagicCircle
- Ready occupancy
- transition circle
- Close / Open presentation entry point
- circle-relative transform conversion

UMAWorldTransitionMaskComponent
- 실제 Close / Open presentation 구현
~~~

현재 기능은 이 구조로 동작한다.

문제는 기능 동작 여부가 아니라, TransitionSubsystem이 Space streaming 세부 구현까지 직접 소유하고 AMASpace의 Prepare가 실제 준비 작업보다 검증 관문에 가까운 형태라는 점이다.

## 3. 수정 후 가장 큰 코드 구조

목표 구조는 다음처럼 단순하게 잡는다.

~~~text
External caller
    |
    v
UMASpaceTransitionSubsystem
    |
    | "이 Space를 로드해"
    v
UMASpaceLoader
    |
    | level streaming
    v
AMASpace
    |
    | 실제 필요한 작업의 완료 신호
    v
UMASpaceTransitionSubsystem
    |
    v
Close -> Handoff -> Open
~~~

각 객체의 한 문장 책임:

~~~text
UMASpaceTransitionSubsystem
= Space A에서 Space B로 넘어가는 순서를 소유한다.

UMASpaceLoader
= MapAsset을 현재 UWorld에 로드하고 결과 AMASpace를 반환하며, 해당 Space를 unload한다.

AMASpace
= 로드된 Space 하나를 대표하는 결과 객체다.

AMAPlayerControllerBase
= Server Subsystem과 Client Subsystem 사이 RPC를 운반한다.

AMAMagicCircle
= Space transition의 기준 Circle과 자기 presentation entry point를 제공한다.
~~~

## 4. UMASpaceTransitionSubsystem

UMASpaceTransitionSubsystem은 UWorldSubsystem을 유지한다.

현재 Persistent WorldRoot와 정확히 수명을 공유하며, 특정 streamed Space에 속하지 않고 여러 Space 사이의 transition을 조율하기 때문이다.

외부 진입점은 현재처럼 작게 유지한다.

~~~text
RequestTransition(...)
~~~

Subsystem 코드에서 읽혀야 하는 것은 transition 순서다.

목표 흐름:

~~~text
RequestTransition
-> Loader에 Destination load 요청
-> Destination AMASpace 수신
-> 현재 필요한 완료 조건 대기
-> Client readiness 수집
-> Close
-> Handoff
-> Open
-> Loader에 Source unload 요청
-> Finish
~~~

Subsystem 안에서 제거할 구현 세부사항:

- ULevelStreamingDynamic 직접 호출
- SetShouldBeLoaded / SetShouldBeVisible
- SetIsRequestingUnloadAndRemoval
- streaming level lifetime bookkeeping 자체
- loaded level에서 Space를 얻기 위한 기술 세부 구현

이것들은 SpaceLoader가 닫는다.

Subsystem은 "로드해", "언로드해"라는 의미만 전달한다.

## 5. UMASpaceLoader

첫 버전에는 Loader 하나만 추가한다.

Loader를 위한 LoaderManager, Registry, Factory, Handle hierarchy를 추가하지 않는다.

Loader가 Transition에서만 사용되는 동안에는 TransitionSubsystem이 소유하는 내부 UObject 정도면 충분하다.

개념 public contract:

~~~text
LoadSpace(SpaceMap, InstanceTransform, InstanceIdentity, OnLoaded)
UnloadSpace(AMASpace)
~~~

정확한 함수명은 구현 시 현재 코드 스타일에 맞춘다.

LoadSpace의 의미:

~~~text
requested map streaming 시작
-> 해당 streaming instance load 완료
-> 그 loaded level을 대표하는 AMASpace 하나를 얻음
-> OnLoaded(AMASpace)
~~~

Loader의 산출물은 ULevelStreaming이 아니라 AMASpace다.

ULevelStreaming은 Loader 내부 구현 세부사항이다.

### Space Owner 획득

현재 AMASpace가 BeginPlay에서 TransitionSubsystem에 자신을 등록하는 경로는 Destination load 결과를 역방향으로 전달하기 위한 장치다.

Loader가 specific loaded level의 결과를 직접 받을 수 있다면 이 등록 경로는 제거한다.

첫 구현에서는 loaded level 하나의 Actor 목록에서 AMASpace를 한 번 확인하여 정확히 하나를 얻는 방식도 허용한다.

이것은 global Actor search나 반복 Registry 탐색이 아니다.

~~~text
specific streaming instance
-> its loaded ULevel
-> exactly one AMASpace
~~~

Loader가 자신의 산출물을 얻기 위해 수행하는 제한된 lookup이다.

0개 또는 2개 이상이면 Loader가 유효한 AMASpace 결과를 만들 수 없으므로 load 실패다.

별도 Validator 객체는 만들지 않는다.

### Initial Current Space

WorldRoot가 시작 시 LobbyHubMap을 이미 blocking streaming하는 현재 bootstrap 경로는 Loader가 한 번 Adopt하는 방식으로 확정한다.

목표 흐름:

~~~text
WorldRoot 시작
-> 이미 loaded 상태인 Streaming Level들을 Loader가 한 번 확인
-> AMASpace를 포함한 Level을 정확히 하나 획득
-> Loader가 AMASpace <-> ULevelStreaming 관계를 인수
-> Initial AMASpace 반환
-> TransitionSubsystem이 CurrentSpace로 설정
~~~

이 탐색은 bootstrap 시 한 번만 수행한다.

전환마다 World 전체를 다시 검색하거나 Registry를 조회하는 구조로 확장하지 않는다.

이 방식으로 다음을 제거한다.

- AMASpace::BeginPlay()의 TransitionSubsystem 자기 등록
- AMASpace::EndPlay()의 TransitionSubsystem 자기 해제
- RegisterSpace / UnregisterSpace
- Initial Space 전용 Registry / Provider / Bootstrap Actor

초기 Space는 Adopt, 이후 Space는 Load라는 차이만 있고 결과는 둘 다 AMASpace다.

~~~text
AdoptInitialSpace() -> AMASpace
LoadSpace(...)       -> AMASpace
~~~

TransitionSubsystem은 결과를 받은 이후 둘을 구분하지 않는다.

## 6. AMASpace는 Loader의 산출물

AMASpace의 핵심 의미를 다음으로 고정한다.

> 현재 UWorld 안에 로드된 하나의 Space instance를 대표하는 객체.

AMASpace가 현재 제공해야 하는 값은 실제 transition에서 사용되는 Magic Circle 참조 하나로 최소화한다.

현재 확정:

- TransitionCircle

현재 AMASpace 자신의 Transform은 사용되지 않고 TransitionCircle도 별도 Actor 참조이므로, 단지 RootComponent를 만들기 위한 SceneRoot와 생성자는 두지 않는다.

~~~text
AMASpace
-> TransitionCircle
~~~

MASpace.cpp에 다른 실제 동작이 없다면 별도 .cpp도 필요하지 않다.

기존 TransitionAnchor는 타입이 AMAMagicCircle임을 이름만 보고 유추하기 어렵기 때문에 TransitionCircle로 변경한다.

제거:

- SpaceIdentity
- Bounds

SpaceIdentity는 현재 어떤 식별 의미를 가져야 하는지 확정되지 않았고 실제 소비자도 없다.

Bounds도 현재 transition에서는 사용되지 않는다. 이후 PCG / Battle generation에 실제 generation bounds가 필요해지면 그 기능의 구체적인 요구와 owner를 보고 추가한다.

AMASpace가 미래의 모든 Space-local 데이터를 미리 보유하는 객체가 되지 않게 한다.

AMASpace가 소유하지 않을 것:

- level streaming 시작/종료
- transition request
- client readiness
- RequestId
- Battle procedural generation orchestration
- generic validation pipeline

Loader가 AMASpace를 결과로 반환하고 TransitionSubsystem은 AMASpace를 대상으로 transition을 수행한다.

## 7. Prepare() / Prepared 제거

현재 구현의:

~~~text
Loaded
-> Prepare()
-> Prepared
-> Active
-> Retiring
~~~

에서 Prepare()는 실제 준비 작업보다 다음 역할에 가깝다.

- TransitionCircle null 확인
- lifecycle state 변경
- bool 결과 반환

이 검증 단계를 위해 Prepare abstraction을 유지하지 않는다.

목표:

~~~text
Map load
-> AMASpace 산출
-> 실제 필요한 작업 수행
-> 해당 작업 owner가 완료 신호
-> transition
~~~

따라서 현재 first implementation에서는 다음을 제거 대상으로 본다.

- AMASpace::Prepare()
- EMASpaceLifecycle::Prepared
- "Prepare 완료 == DestinationReady"라는 계약
- Prepare Context / Prepare Result 같은 새 추상화

TransitionCircle이 실제 handoff 시 필요한데 없으면 그 사용 지점에서 ensure/failure 처리한다.

아직 발생하지 않은 문제를 미리 검사하기 위한 별도 validation phase는 만들지 않는다.

## 8. Generic Space Ready도 지금 만들지 않는다

Prepare를 제거한다고 즉시 다음을 만들지 않는다.

- FMASpacePreparationContext
- FMASpacePreparationResult
- generic OnSpaceReady aggregator
- generic readiness dependency list
- Ready Validator
- Preparing state machine

완료 신호는 실제 작업의 owner가 보낸다.

현재 transition test 범위:

~~~text
SpaceLoader load 완료
-> Destination AMASpace 확보
-> 현재 필요한 destination load 조건 완료
~~~

향후 Battle generation 연결 시:

~~~text
SpaceLoader load 완료
-> AMASpace 확보

LevelManager GenerateBattle(...)
-> 실제 생성 완료
-> BattleReady
~~~

TransitionSubsystem은 그 시점에 실제로 필요한 완료 신호만 기다린다.

"모든 미래 준비 작업"을 표현하는 generic SpaceReady framework는 실제 공통 요구가 생길 때 추가한다.

## 9. Lifecycle도 최소화

현재 AMASpace의 lifecycle enum은 실제 동작보다 상태 기록 역할이 크다.

첫 구현에서 lifecycle 자체가 기능을 만들지 않는다면 상태 enum을 유지하기 위해 코드를 추가하지 않는다.

최소 기준:

~~~text
TransitionSubsystem
- CurrentSpace
- DestinationSpace
~~~

이 두 참조만으로 현재 transition에 필요한 공간 역할은 이미 표현된다.

Activate / Retire에 실제 Space-local 동작이 생기는 시점:

- MusicTag activation
- current-only presentation
- Battle local gameplay enable/disable
- 기타 실제 Space-local policy

그때 AMASpace에 의미 있는 Activate / Retire entry point를 추가한다.

단순히 미래를 위해 enum 값만 바꾸는 lifecycle scaffold는 만들지 않는다.

## 10. GenerationSeed는 유지

FMASpaceTransitionRequest의 GenerationSeed는 현재 transition test에서는 사용되지 않더라도 제거하지 않는다.

03 Runtime Generation Rebuild Mockup에서 확정된 다음 연결 범위이며, Server/Client가 동일한 generation request를 식별하는 실제 network input이다.

다만 Loader는 GenerationSeed를 알 필요가 없다.

~~~text
SpaceLoader
- SpaceMap
- InstanceTransform
- streaming instance identity

LevelManager
- Destination AMASpace
- GenerationSeed / GenerationSettings
~~~

서로 다른 책임의 입력을 하나의 Loader API에 억지로 넣지 않는다.

## 11. PlayerController RPC bridge

현재 AMAPlayerControllerBase의 역할은 유지한다.

~~~text
Server UMASpaceTransitionSubsystem
-> PlayerController Client RPC
-> Client UMASpaceTransitionSubsystem

Client UMASpaceTransitionSubsystem
-> PlayerController Server RPC
-> Server UMASpaceTransitionSubsystem
~~~

PlayerController는 transition policy를 갖지 않는다.

현재 단계에서 별도 NetworkManager / RPCComponent를 만들지 않는다.

실제 network boundary가 PlayerController 하나로 충분하다.

## 12. 현재 구현에서 우선 제거/이동할 코드

### TransitionSubsystem에서 Loader로 이동

- LoadDestination()
- ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr 호출
- Source/Destination streaming instance unload 세부 구현
- streaming instance와 AMASpace 결과 연결에 필요한 기술 코드

### AMASpace에서 제거 검토

- 사용되지 않는 SceneRoot
- SceneRoot 생성만을 위한 AMASpace 생성자
- 다른 구현이 없다면 MASpace.cpp
- BeginPlay / EndPlay의 TransitionSubsystem registration
- Prepare()
- Prepared lifecycle
- GetOwningStreamingLevel()이 Loader 내부 tracking으로 대체 가능하면 제거
- 실제 기능 없이 enum만 바꾸는 lifecycle state

### 유지

- RequestTransition()
- FMASpaceTransitionRequest / RequestId
- server/client milestone 통신
- PendingPlayers를 이용한 실제 phase barrier
- Close / Handoff / Open orchestration
- circle-relative player transform handoff
- MagicCircle presentation entry point
- GenerationSeed

## 13. MagicCircle 간 Closed 상태 이전

현재 MoveClosedTransitionTo()처럼 한 AMAMagicCircle이 다른 AMAMagicCircle의 transition state를 직접 조작하는 구조는 별도 수정 대상으로 둔다.

책임 기준:

~~~text
UMASpaceTransitionSubsystem
= 어떤 Circle을 언제 Close / Closed restore / Open할지 순서를 소유

AMAMagicCircle
= 자기 자신의 transition presentation state만 소유
~~~

목표 방향:

~~~text
TransitionSubsystem
-> Source TransitionCircle Close
-> player handoff
-> Source TransitionCircle의 Closed 표현 해제
-> Destination TransitionCircle을 Closed 상태로 준비
-> Destination TransitionCircle Open
~~~

Source Circle이 Destination Circle을 직접 제어하지 않는다.

Source와 Destination의 Closed 표현은 동시에 활성화하지 않는다. 현재 Stencil 대상 ownership 때문에 Source Closed 표현을 먼저 해제한 뒤 Destination Closed 표현을 설정한다.

이 순서는 TransitionSubsystem이 조율한다. 첫 구현에서는 Mask 간 Transfer 객체나 원자적 transfer abstraction을 추가하지 않는다. 실제로 같은 프레임의 순차 호출만으로 해결되지 않는 문제가 확인될 때만 별도 transfer 계약을 검토한다.

정확한 함수명은 구현 시 최소 API로 정한다.

예:

~~~text
CloseTransition()
SetTransitionClosed()
OpenTransition()
~~~

이 문제는 SpaceLoader / AMASpace 분리와 책임 축이 다르므로 구현과 검수도 별도 변경 단위로 유지한다.

## 14. Camera Director의 미사용 Blend 완료 추상화 제거

현재 구현에서 일반 ViewTarget 전환 코드는 하나의 내부 함수로 합치는 것 자체는 유효하다.

~~~text
BlendToViewTarget(ViewTarget, BlendTime)
~~~

다만 현재 호출자는 Blend 완료 후 실행할 작업을 전달하지 않는다.

따라서 첫 구현에서는 다음 callback machinery를 두지 않는다.

- FSimpleDelegate OnBlended 파라미터
- ViewTargetBlendFinishedDelegate
- ViewTargetBlendCompleteHandle
- HandleViewTargetBlendComplete()
- CancelViewTargetBlendCompletion()
- APlayerCameraManager::OnBlendComplete() 등록/해제 코드

목표 형태:

~~~text
SwitchToViewTarget(...)
-> BlendToViewTarget(ViewTarget, BlendTime)

SwitchToPawnCamera(...)
-> BlendToViewTarget(Pawn, BlendTime)
~~~

Blend 완료 후 실제 후속 작업이 생길 때만 completion callback을 추가한다.

## 15. 만들지 않을 것

이번 feedback 구현에서 다음은 추가하지 않는다.

- SpaceLoaderSubsystem
- SpaceRegistry
- SpaceFactory
- SpaceHandle wrapper hierarchy
- generic SpacePreparationContext
- generic SpacePreparationResult
- generic ReadinessAggregator
- Space Validator class
- generic lifecycle framework
- generic async job framework
- cancellation token framework
- transition step object hierarchy
- strategy/interface hierarchy
- Loader를 위한 별도 Manager 계층

필요한 실제 문제가 생긴 뒤 추가한다.

## 16. 구현 후 코드가 읽혀야 하는 형태

### MASpaceLoader.cpp

~~~text
Load 요청
-> Level streaming
-> specific loaded level에서 AMASpace 결과 획득
-> callback

Unload 요청
-> 해당 streaming instance 제거
~~~

여기에 transition phase, player RPC, MagicCircle 정책이 나오면 안 된다.

### MASpace

~~~text
이 객체는 하나의 loaded Space를 대표한다.
-> TransitionCircle
~~~

현재는 자체 Transform이나 별도 runtime 동작이 없으므로 SceneRoot와 생성자를 요구하지 않는다.

헤더만으로 완결되면 MASpace.cpp를 만들지 않는다.

실제 Space-local behavior가 생기기 전에는 더 많은 lifecycle scaffold를 요구하지 않는다.

### MASpaceTransitionSubsystem.cpp

~~~text
RequestTransition
-> Loader.LoadSpace
-> Destination Space 받음
-> 필요한 completion을 기다림
-> Close
-> Handoff
-> Open
-> Loader.UnloadSpace
-> Finish
~~~

코드 위에서 transition 순서가 바로 읽혀야 한다.

## 17. Naming Contract

이번 리팩토링에서 살아남는 이름은 처음 보는 사람이 타입과 역할을 바로 유추할 수 있도록 정리한다.

확정 변경:

~~~text
TransitionAnchor
-> TransitionCircle

GetTransitionAnchor()
-> GetTransitionCircle()

UMAWorldTransitionMaskComponent
-> UMASpaceTransitionMaskComponent

UMAWorldTransitionVisibilityComponent
-> UMASpaceTransitionVisibilityComponent

FMASpaceRequest
-> FMASpaceTransitionRequest

FMASpaceTransitionRequest::MapAsset
-> DestinationMap

FMASpaceTransitionRequest::DestinationSlot
-> DestinationSlotTransform


ToCircleSpace()
-> WorldToCircleTransform()

ToWorldSpace()
-> CircleToWorldTransform()

Sound.WorldTransition.Close
-> Sound.SpaceTransition.Close

Sound.WorldTransition.Open
-> Sound.SpaceTransition.Open
~~~

유지:

~~~text
UMASpaceTransitionSubsystem
UMASpaceLoader
AMASpace
AMAMagicCircle
RequestTransition()
LoadSpace()
UnloadSpace()
GenerationSeed
RequestId
EMASpaceTransitionClientMilestone
DestinationReady / Closed / Opened
~~~

네이밍 원칙:

- 같은 UWorld 안의 Space 전환이므로 새 코드에 WorldTransition 이름을 남기지 않는다.
- 실제 타입이 AMAMagicCircle이면 추상적인 Anchor보다 Circle을 사용한다.
- Request 구조체는 어떤 요청인지 이름 자체로 드러낸다.
- Transition request 안에서는 DestinationMap / DestinationSlotTransform처럼 목적지 역할을 드러낸다.
- Loader 안에서는 SpaceMap / InstanceTransform처럼 transition 정책과 무관한 일반 이름을 사용한다.
- FTransform 값은 이름에서 Transform임을 드러낸다.
- 상태를 내부에 보관하는 초기 Space 인수 함수는 순수 조회처럼 보이는 Find보다 Adopt를 사용한다.
- 좌표 변환 함수는 변환 방향을 함수명에 명시한다.
- 이미 충분히 명확한 이름은 더 길게 만들지 않는다.

## 18. Acceptance Criteria

### 큰 구조

- UMASpaceTransitionSubsystem은 UWorldSubsystem으로 유지된다.
- 외부 transition 진입점은 Subsystem에 있다.
- Loader가 SpaceMap -> AMASpace 변환을 닫는다.
- AMASpace는 Loader의 결과 객체로 취급된다.
- TransitionSubsystem은 ULevelStreaming 세부 API를 직접 다루지 않는다.

### 최소성

- 현재 필요하지 않은 Prepare/Prepared validation layer가 없다.
- generic SpaceReady framework가 없다.
- Loader를 위한 Manager/Registry/Factory hierarchy가 없다.
- 실제 동작 없는 lifecycle state를 유지하기 위한 코드가 없다.
- 실제 current requirement에 없는 abstraction이 추가되지 않는다.
- AMASpace에 사용되지 않는 SceneRoot/생성자 scaffold가 없다.
- Camera Director에 현재 소비자가 없는 ViewTarget blend completion callback machinery가 없다.

### 동작 유지

- 현재 검증된 same-UWorld seamless TP가 그대로 동작한다.
- RequestId 기반 stale response rejection을 유지한다.
- Server/Client destination load barrier를 유지한다.
- Close -> Handoff -> Open 순서를 유지한다.
- Source Space unload를 유지한다.
- TransitionCircle-relative player transform을 유지한다.

### 다음 기능 연결

Battle generation을 연결할 때 큰 구조를 다시 뜯지 않는다.

~~~text
Loader
-> Destination AMASpace

LevelManager
-> GenerateBattle(DestinationSpace, Settings)
-> BattleReady

TransitionSubsystem
-> BattleReady 포함 실제 필요한 completion을 기다림
-> transition
~~~

이 연결을 위해 지금 generic preparation framework를 미리 만들지 않는다.

## 19. 구현 원칙 요약

~~~text
TransitionSubsystem
= 요청과 순서

SpaceLoader
= Map -> AMASpace / unload

AMASpace
= loaded Space result
= 현재는 TransitionCircle만 보유

LevelManager
= 실제 Battle generation

각 실제 작업 owner
= 자기 작업이 끝나면 completion signal
~~~

첫 버전의 목표는 미래의 모든 경우를 표현하는 것이 아니다.

현재 기능을 정확한 책임으로 나누고, 작동에 필요하지 않은 코드와 추상화를 덜어내며, 다음 확정 기능이 들어와도 큰 책임 경계를 뒤집지 않는 것이다.
