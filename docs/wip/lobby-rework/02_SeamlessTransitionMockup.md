# 02. Persistent Space Transition Mockup

> Status: Mockup-ready ownership contract
> Branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8

## 1. 목적

Lobby와 Runtime Generated Battle Field 사이를 끊김 없이 전환하면서도 Hub와 Battle은 각각 독립된 .umap으로 유지한다.

기존 Seamless Travel 패키지 테스트에서 UWorld 교체 자체와 PSO/렌더 준비에 의한 hitch가 확인되었다.

따라서 정상 전환 경로에서는 UWorld를 교체하지 않는다.

핵심 원칙:

> 사용처는 의미만 말한다.
>
> 기능의 주인이 정책, 검증, 상태, 실행을 닫는다.
>
> 문제가 생기면 기능 이름을 따라 한곳만 보면 원인을 추적할 수 있어야 한다.

## 2. 최종 구조

~~~text
Persistent WorldRoot
|
+-- Current streamed Space
|
+-- Destination streamed Space
|
+-- SpaceTransition
|
+-- LevelManager
~~~

Hub와 Battle은 독립 .umap으로 편집한다.

런타임에서는 같은 Persistent UWorld 안의 서로 떨어진 Slot에 스트리밍한다.

정상 경로에서 다음에 의존하지 않는다.

- ServerTravel
- TransitionMap
- Hub/Battle을 한 거대한 .umap에 합치는 방식
- Mask Closed 상태를 실제 로딩 화면으로 사용하는 방식

## 3. 책임 구조

최종 책임은 다음처럼 나눈다.

~~~text
MagicCircle
- 파티 집결 / Ready 정책
- 출발 요청
- Transition Restricted 요청
- Sphere Mask / 전환 연출

SpaceTransition
- SpaceRequest 생성
- Destination streaming
- Server / Client 준비 확인
- Current / Destination 관리
- Anchor-relative player handoff
- Source retirement / unload

Space
- 해당 streamed 공간의 정체성
- Bounds
- Transition Anchor
- Space-local references
- lifecycle state
- Prepare
- Activate
- Retire

LevelManager
- 지정된 Battle Space의 생성
- Generation Settings 적용
- 재생성 정책
- BattleReady

Generation Feature Owner
- 자기 기능의 계산
- 자기 데이터
- 자기 runtime resource
- 자기 Clear / Replace / Rebuild
~~~

GameMode는 위 규칙을 직접 구현하지 않는다.

GameMode는 session-level rule owner로 남고 SpaceTransition이나 LevelManager의 세부 정책을 대신 수행하지 않는다.

## 4. Space Owner는 streamed .umap마다 정확히 하나

각 streamed .umap에는 해당 공간을 대표하는 명확한 Space Owner가 하나 있어야 한다.

개념 계약:

~~~text
Space
- Identity
- SpaceType
- Bounds
- TransitionAnchor
- LifecycleState
- Space-local references

Prepare()
Activate()
Retire()
~~~

정확한 클래스명은 구현 시 결정한다.

중요한 것은 이름이 아니라 다음 계약이다.

> 외부 시스템은 streamed level 내부 Actor를 다시 찾아다니지 않는다.

SpaceTransition이나 LevelManager가 다음을 반복하면 안 된다.

~~~text
Find MagicCircle
Find Bounds
Find Encounter
Find WaveManager
Find activation targets
Find objects to unload
~~~

해당 공간의 필요한 참조는 Space Owner가 닫는다.

## 5. Space 등록 계약

SpaceTransition은 글로벌 Actor 검색으로 Space를 찾지 않는다.

streamed .umap이 로드되면 그 인스턴스의 Space Owner가 명시적인 등록 경로를 통해 자신을 등록한다.

개념:

~~~text
Streamed level instance loaded
-> its Space Owner registers
-> SpaceTransition receives Space handle
-> handle is bound to SpaceRequestId / streaming instance
~~~

정확한 구현 수단은 목업에서 선택한다.

가능한 수단:

- Level root registration
- known Space Owner actor registration
- LevelScriptActor based registration
- 명시적 interface/component registration

어떤 수단을 쓰더라도 정상 경로에서 반복적인 GetAllActors / FindActor 탐색에 의존하지 않는다.

## 6. Space가 소유하는 것과 소유하지 않는 것

Space는 공간 lifecycle의 주인이다.

소유/대표:

- Identity
- Bounds
- Transition Anchor
- Space-local Actor references
- local presentation activation target
- lifecycle state
- Prepare / Activate / Retire policy

Space가 Battle procedural generation 구현을 직접 가져가지는 않는다.

~~~text
Space
= 공간 lifecycle owner

LevelManager
= Battle generation lifecycle owner
~~~

예를 들어 Battle Space가 Loaded 된 뒤 실제 지형 생성을 수행하는 것은 LevelManager다.

## 7. Space Lifecycle

기본 상태:

~~~text
Loading
-> Loaded
-> Preparing
-> Prepared
-> Active
-> Retiring
-> Unloaded
~~~

### Prepare

Space 자체가 transition 대상이 될 수 있도록 공간-local 준비를 닫는다.

예:

- required local references 확인
- local presentation 준비
- Anchor / Bounds 유효성 검증
- Space-specific Actor 초기화

Battle procedural generation은 LevelManager가 별도로 수행한다.

### Activate

해당 Space를 Current Space로 전환할 때 필요한 local activation을 닫는다.

예:

- current-only presentation 활성화
- space-local gameplay activation
- 필요한 local interaction 활성화

### Retire

Source Space가 더 이상 Current가 아닐 때 공간-local 종료 정책을 닫는다.

예:

- current-only gameplay 비활성화
- local effects 정리
- unload 가능한 상태로 전환

실제 streamed level unload 요청은 SpaceTransition이 수행한다.

## 8. Runtime Space Slot

각 Space Map은 독립적으로 제작한다.

런타임에서는 서로 겹치지 않는 Slot에 배치한다.

~~~text
Slot A
[ Current Space ]

---------------- separated ----------------

Slot B
[ Destination Space ]
~~~

기본 모델은 Current / Destination double buffer다.

Slot separation은 예상 Bounds + safety margin을 기준으로 정한다.

하드코딩된 특정 거리 자체를 architecture contract로 두지 않는다.

## 9. SpaceRequest 네트워크 계약

각 Destination 준비 요청을 식별하는 최소 네트워크 값:

~~~text
SpaceRequest
- SpaceRequestId
- MapAsset
- SlotTransform
- GenerationSeed
- GenerationSettingsSnapshot or SettingsIdentity
~~~

SpaceRequest는 불필요한 Context 객체가 아니다.

멀티플레이에서 다음을 식별하기 위한 필수 경계다.

- 어느 destination instance인가
- 어느 Slot인가
- 어느 Seed/Settings인가
- 어느 ClientReady가 어느 요청에 대한 응답인가

## 10. SpaceTransition 책임

SpaceTransition은 긴 전환 경로를 한 이름 아래 닫는다.

외부 사용처가 streaming 순서나 readiness 조건을 조립하지 않는다.

SpaceTransition이 소유하는 정책:

- SpaceRequestId 발급
- Destination stream request
- streamed Space 등록 수신
- Destination Space Prepare 요청
- LevelManager에 Battle generation 요청
- ServerDestinationReady 판단
- ClientReady 수집
- PartyDestinationReady 판단
- transition 가능 여부
- player Anchor handoff
- Current/Destination swap
- Source Retire
- streamed Source unload

## 11. MagicCircle 책임

MagicCircle은 Space streaming을 직접 관리하지 않는다.

MagicCircle이 담당:

- 누가 Circle 안에 있는가
- Party Ready 정책
- 출발 요청
- Transition Restricted 상태 시작/해제와 연출 협력
- Sphere Mask close/open presentation

MagicCircle이 SpaceTransition에 전달하는 의미는 단순해야 한다.

예:

~~~text
Party is ready.
Request departure.
~~~

그 이후:

- Destination이 준비됐는가
- Client가 준비됐는가
- SpaceRequest가 최신인가
- 지금 handoff 가능한가

는 SpaceTransition이 판단한다.

## 12. Server / Client Prepare

Server가 Destination을 결정하면 SpaceRequest를 전달한다.

Client:

~~~text
Receive SpaceRequest
-> stream requested .umap
-> receive/register its Space Owner
-> Space.Prepare()
-> prepare required local representation
-> apply authoritative GenerationSeed / Settings
-> report ClientReady(SpaceRequestId)
~~~

stale SpaceRequestId의 Ready는 무시한다.

## 13. DestinationReady

~~~text
ServerDestinationReady
=
Destination Space prepared
AND
Battle generation ready if Battle Space
AND
required server-side resources ready

PartyDestinationReady
=
ServerDestinationReady
AND
all required clients reported ClientReady(CurrentSpaceRequestId)
~~~

정상 transition gate:

~~~text
AllPlayersReady
AND
PartyDestinationReady
~~~

## 14. Destination Prepare First

플레이어가 Hub에 있는 동안 Destination을 준비한다.

~~~text
SpaceTransition creates SpaceRequest
-> stream Destination .umap
-> Destination Space registers
-> Space.Prepare()
-> if Battle:
     LevelManager.GenerateFor(DestinationSpace, Settings)
-> server resources ready
-> clients ready
-> PartyDestinationReady
~~~

핵심:

> Mask는 loading screen이 아니다.

목적지가 준비된 뒤에만 실제 transition presentation을 시작한다.

## 15. Transition Restricted

전환 시작 이후:

- Magic Circle 내부 이동 가능
- 전환용 고정 저속
- 일반 이동속도 buff/debuff 무시
- skill 불가
- dash/teleport/jump 등 탈출 가능 행동 제한
- interaction 불가
- Ready 취소 불가
- SafeRadius 밖으로 나갈 수 없음
- handoff 순간에만 짧은 full lock 허용

MagicCircle이 Character 내부 능력 정책을 직접 매 프레임 조작하지 않는다.

플레이어 쪽 실제 이동/능력 owner가 제한 상태를 집행한다.

## 16. Sphere Transition

시각적 계약:

~~~text
Large Radius
-> Radius Down
-> Closed
-> player handoff
-> Radius Up
-> Destination revealed
~~~

world-space spherical transition 결과가 핵심이다.

Post Process Sphere Mask는 1순위 후보지만 architecture contract로 고정하지 않는다.

## 17. Player Handoff

중앙 정렬하지 않는다.

전환 직전:

~~~text
PlayerLocalTransform =
PlayerWorldTransform relative to SourceSpace.TransitionAnchor
~~~

Closed 후:

~~~text
DestinationWorldTransform =
PlayerLocalTransform applied to DestinationSpace.TransitionAnchor
~~~

정상 경로에서:

- 파티 상대 배치 유지
- 위치/방향 유지
- 중앙 snap 없음
- 주변만 교체된 것처럼 보임

을 만족해야 한다.

## 18. Transition Lifecycle

~~~text
1. Current Space Active
2. SpaceTransition creates SpaceRequest
3. Destination streaming
4. Destination Space registers
5. Destination Space Prepare
6. LevelManager generates Battle if required
7. ServerDestinationReady
8. Clients finish same SpaceRequest and report Ready
9. PartyDestinationReady
10. MagicCircle reports AllPlayersReady / DepartureIntent
11. transition delay or activation sequence
12. TransitionRestricted
13. Sphere closes
14. snapshot Anchor-relative player transforms
15. brief movement lock
16. SpaceTransition moves players
17. Destination Space Activate
18. Current/Destination swap
19. Sphere opens
20. TransitionRestricted released
21. Source Space Retire
22. SpaceTransition unloads Source
~~~

## 19. Persistent / Space-local ownership

### Persistent

- GameMode / GameState
- session state
- SpaceTransition
- LevelManager

### Space-local

- Space Owner
- MagicCircle
- Encounter/Wave owner
- Space-specific objective actors
- local presentation actors

Persistent GameMode에 Space-specific 정책을 누적하지 않는다.

## 20. World-global presentation

멀리 떨어진 Destination도 다음 요소로 Current Space에 영향을 줄 수 있다.

- DirectionalLight
- SkyAtmosphere
- SkyLight
- VolumetricCloud
- HeightFog
- global audio
- global post process

Destination Prepare 중 Current Space의 global presentation을 바꾸지 않는다.

필요한 global activation policy는 Space Activate 또는 별도 global presentation owner 중 실제 구조에 맞는 한 주인이 닫는다.

두 군데에서 동시에 제어하지 않는다.

## 21. Performance Probe

Persistent Streaming은 UWorld swap을 없애지만 다음 비용은 남는다.

- level streaming
- component registration
- procedural generation
- Dynamic Mesh
- collision
- PCG
- Nav
- render resources
- PSO
- texture streaming

Probe에서는 총 준비 시간보다 Current Space의 max frame time을 우선 확인한다.

실제 병목이 확인되면 병목 기능의 주인 안에서 먼저 해결한다.

범용 scheduler는 실제 공통 요구가 확인되기 전까지 만들지 않는다.

## 22. Acceptance Criteria

### Ownership

- streamed .umap마다 정확히 하나의 Space Owner가 존재한다.
- SpaceTransition/LevelManager가 level 내부 Actor를 반복 검색하지 않는다.
- Space Owner가 Anchor/Bounds/local references/lifecycle을 닫는다.
- MagicCircle, SpaceTransition, Space, LevelManager 책임이 겹치지 않는다.

### Network

- SpaceRequestId로 요청을 식별한다.
- ClientReady는 SpaceRequestId와 함께 검증한다.
- stale Ready는 무시된다.
- PartyDestinationReady 전에는 transition하지 않는다.

### Transition

- AllPlayersReady + PartyDestinationReady가 gate다.
- Anchor-relative transform을 유지한다.
- same UWorld에서 handoff한다.
- Source는 Retire 후 독립 unload 가능하다.

### Runtime Generation

- Battle .umap Loaded와 BattleReady를 구분한다.
- LevelManager가 지정된 Battle Space를 생성한다.
- Space가 procedural generation 정책을 직접 구현하지 않는다.

## 23. Deferred

- final Magic Circle VFX
- final transition sound
- Map Settings UI
- final sky/lighting art ownership
- late join UX
- dedicated server optimization
- multiple destination cache
- exact class naming
- advanced timeout/recovery UX

## 24. 핵심 요약

~~~text
MagicCircle
  -> Ready / DepartureIntent

SpaceTransition
  -> SpaceRequest
  -> Streaming
  -> ClientReady
  -> Destination readiness
  -> Anchor handoff
  -> Current/Destination swap
  -> Source unload

Space
  -> Identity / Bounds / Anchor
  -> local references
  -> Prepare / Activate / Retire

LevelManager
  -> Battle generation / rebuild policy

Generation Feature Owner
  -> own calculation / data / runtime resource
~~~

사용처는 의미만 말하고, 실제 정책과 실행은 해당 기능의 주인이 닫는다.
