# 03. Runtime Generation Rebuild Mockup

> Status: Mockup-ready ownership and staged implementation contract
> Branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8

## 1. 목적

Runtime Generated Battle Space가 최초 생성뿐 아니라 이후 Generation Settings 변경에 따라 재구축될 수 있도록 한다.

초기 버전에는 사용자용 Map Settings UI가 없다.

재구축 기능 자체는 현재 구현 범위에 포함한다.

다만 아직 존재하지 않는 문제를 위해 범용 Dirty/Revision/Scheduler framework를 먼저 만들지 않는다.

핵심 원칙:

> 사용처는 무엇이 바뀌었는지만 말한다.
>
> LevelManager가 어떤 생성 결과를 무효화하고 어떤 순서로 다시 만들지 닫는다.
>
> 각 Generation Feature Owner는 자기 기능의 실제 구현과 runtime resource를 닫는다.

## 2. 책임 구조

~~~text
External caller
- 의미 있는 설정 변경만 요청

LevelManager
- Battle generation policy
- rebuild policy
- dependency/order
- BattleReady
- Target Battle Space

Generation Feature Owner
- 자기 기능 계산
- 자기 데이터
- 자기 runtime resource
- 자기 Clear / Replace / Rebuild
~~~

외부 호출자가 여러 Rebuild 함수를 조립하지 않는다.

외부 호출자가 BattleReady를 직접 바꾸지 않는다.

## 3. Generation Feature Owner

모든 기능을 별도 Manager 객체로 만들 필요는 없다.

기능의 주인은 실제 구조에 따라 다음 중 하나일 수 있다.

- Manager
- Component
- Builder
- Service
- Actor
- 단순 C++ object

중요한 기준:

> 해당 기능을 수정하거나 디버깅할 때 한 주인만 보면 끝나야 한다.

따라서 기존 문구인:

~~~text
Lower manager decides how.
~~~

를 객체 형태의 강제 계약으로 사용하지 않는다.

정확한 계약:

> LevelManager는 Battle 공간 생성/재생성의 순서와 정책을 닫는다.
>
> 각 실제 Generation Feature Owner는 자기 기능 구현을 닫는다.

## 4. GenerationSettings Snapshot

생성 입력은 하나의 Snapshot으로 묶는다.

개념:

~~~text
GenerationSettings
- Seed
- Map / Site settings
- Connection settings
- Path settings
- Terrain settings
- Environment settings
- PCG settings
- other generation-relevant settings
~~~

Feature Owner가 UI나 외부 mutable state를 제각각 직접 읽지 않는다.

한 번의 Generate/Rebuild는 LevelManager가 전달한 확정된 설정을 사용한다.

## 5. 초기 생성 Public Contract

사용처는 생성 내부 순서를 알지 않는다.

예:

~~~text
LevelManager.GenerateBattle(TargetSpace, GenerationSettings)
~~~

LevelManager 내부:

~~~text
BattleReady = false
-> Area feature
-> Connection / Path feature
-> Floor feature
-> Environment feature
-> PCG feature
-> Collision / Nav related owners
-> validate required results
-> BattleReady = true
~~~

정확한 내부 단계는 실제 구현에 따라 달라질 수 있다.

외부 호출 계약은 바뀌지 않는다.

## 6. 설정 변경 Public Contract

외부 사용처가 Rebuild 종류를 선택하지 않는다.

좋은 호출:

~~~text
LevelManager.ApplyPathSettings(NewPathSettings)
LevelManager.ApplyEnvironmentSettings(NewEnvironmentSettings)
LevelManager.ApplyPCGSettings(NewPCGSettings)
LevelManager.ApplyGenerationSettings(NewGenerationSettings)
~~~

실제 public API 개수와 이름은 구현 시 현재 설정 구조에 맞춘다.

공통 원칙:

> 사용처는 변경 의미만 전달한다.

나쁜 호출:

~~~text
Caller:
BattleReady = false
RebuildPath()
RebuildFloor()
RebuildCollision()
RebuildNav()
RegeneratePCG()
BattleReady = true
~~~

이런 orchestration은 금지한다.

## 7. LevelManager가 Rebuild 정책을 닫는다

예: Path 설정 변경

외부:

~~~text
LevelManager.ApplyPathSettings(NewPathSettings)
~~~

LevelManager 내부 정책:

~~~text
BattleReady false
-> Path owner rebuild
-> affected Floor owner update
-> affected Collision/Nav owner update
-> affected PCG owner update
-> validation
-> BattleReady true
~~~

외부 사용처는 Path가 Floor/Nav/PCG에 영향을 준다는 사실을 알 필요가 없다.

## 8. Feature Owner Contract

각 Feature Owner는 자기 구현만 닫는다.

예:

### Area feature owner

- Area data 생성
- Area data 교체
- Area 관련 runtime resource 정리
- 필요한 Area rebuild

### Connection / Path owner

- Connection 계산
- Path 계산
- 자기 데이터 교체

### Floor owner

- Dynamic Mesh 생성/교체
- 자기 geometry resource 관리
- 자기 collision update가 포함되는 구조라면 그 정책까지 소유

### Environment owner

- Environment assignment
- environment state 교체

### PCG owner

- PCG 실행
- Area/local regeneration이 실제로 유효하면 해당 entry point
- 생성 resource lifetime

정확한 객체 타입은 미리 강제하지 않는다.

## 9. Target Battle Space

TargetSpace는 이제 추상적인 값이 아니라 02에서 정의한 Space Owner handle이다.

LevelManager는 Battle Space의 내부 Actor를 다시 찾지 않는다.

예:

~~~text
LevelManager.GenerateBattle(DestinationSpace, Settings)
~~~

DestinationSpace가 제공하는 것:

- Identity
- Bounds
- generation target reference
- 필요한 Space-local reference

LevelManager는 Space의 public contract만 사용한다.

## 10. BattleReady Ownership

BattleReady는 LevelManager가 소유한다.

외부 호출자는 직접 set하지 않는다.

~~~text
LevelManager starts Generate/Rebuild
-> BattleReady false

LevelManager validates required completion
-> BattleReady true
~~~

SpaceTransition은 BattleReady를 변경하지 않고 관찰/조회만 한다.

~~~text
SpaceTransition
-> asks LevelManager whether Destination Battle is ready
~~~

필요성이 확인될 때 event/callback 형태로 전달할 수 있다.

## 11. Full Rebuild

전체 재구축 경로는 LevelManager 내부 안전 fallback으로 유지한다.

개념:

~~~text
LevelManager internal:
RebuildAll(TargetSpace, Settings)
~~~

정상 외부 사용처가 임의로 여러 Feature Owner를 clear/generate하지 않는다.

Full Rebuild 사용 가능 상황:

- Seed / topology 변경
- 영향 범위가 불확실
- partial rebuild 이점이 작음
- 특정 부분 rebuild가 아직 없음
- validation/debug fallback

## 12. Partial Rebuild

부분 재생성 기능은 지금 구현할 수 있다.

하지만 partial rebuild policy는 LevelManager 안에 있다.

예:

~~~text
ApplyEnvironmentSettings(...)
-> LevelManager knows:
   Environment owner rebuild
   -> dependent PCG refresh if required
~~~

현재 실제 dependency를 명시적으로 코드에 적는다.

처음부터 generic dependency graph를 만들지 않는다.

즉:

> Feature-specific Partial Rebuild는 지금 가능.
>
> Generic Dirty Planner는 나중.

## 13. Settings Change During Rebuild

초기에는 동시 변경 UX를 일반화하지 않는다.

재구축 중 새 변경이 들어오지 않는다면 별도 Revision framework가 필요 없다.

필요하면 초기 정책은 간단히 둘 수 있다.

- rebuild 중 settings mutation 거부
- 또는 마지막 요청 하나만 대기

실제 slider 연속 변경 / concurrent rebuild 요구가 생기면 그때:

- Revision
- supersede
- cancellation
- stale callback rejection

을 LevelManager generation lifecycle 위에 추가한다.

Feature ownership은 바뀌지 않는다.

## 14. Performance Policy

목적지 generation/rebuild는 Current Hub와 같은 UWorld에서 수행되므로 hitch는 실제 측정한다.

처음부터 generic Frame Budget Scheduler를 만들지 않는다.

패키지에서 우선 측정:

- total generation time
- max Game Thread frame time
- Render Thread spike
- Dynamic Mesh commit
- collision update
- Nav rebuild
- PCG
- component registration
- PSO/render preparation

실제 병목이 확인되면 기능 이름을 따라 해당 owner부터 본다.

예:

~~~text
Floor hitch
-> Floor owner

Nav hitch
-> Nav-related owner

PCG hitch
-> PCG owner
~~~

문제가 한 기능에 국한되면 그 기능 내부에서 batching/defer를 해결한다.

여러 기능에 공통 scheduler 요구가 생길 때만 generic scheduler를 도입한다.

## 15. Resource Ownership

각 Feature Owner가 자기가 만든 runtime resource를 추적한다.

예:

- Floor owner -> Dynamic Mesh / owned collision resource
- PCG owner -> generated PCG resources
- Environment owner -> environment runtime state
- Nav-related owner -> owned links/request state

LevelManager는 순서와 정책을 소유하지만 모든 세부 resource handle을 중앙에 모으지 않는다.

이 원칙으로 문제 발생 시 해당 기능 주인 하나를 보면 된다.

## 16. 02 SpaceTransition과 연결

정상 흐름:

~~~text
SpaceTransition creates SpaceRequest
-> Destination Space streamed / registered
-> DestinationSpace.Prepare()
-> SpaceTransition requests:
     LevelManager.GenerateBattle(DestinationSpace, Settings)
-> LevelManager BattleReady
-> ServerDestinationReady
-> ClientReady acknowledgements
-> PartyDestinationReady
~~~

설정 변경:

~~~text
External setting source
-> LevelManager.Apply...Settings(...)
-> LevelManager internally invalidates BattleReady
-> required feature owners rebuild
-> LevelManager restores BattleReady
-> SpaceTransition can treat Destination as ready again
~~~

SpaceTransition이 rebuild 종류를 선택하지 않는다.

## 17. Debug / Test Contract

사용자용 Map Settings UI는 아직 없다.

Debug/Test surface도 실제 사용처와 같은 public API를 사용해야 한다.

좋은 테스트:

~~~text
Debug:
LevelManager.ApplyPathSettings(TestPathSettings)
~~~

나쁜 테스트:

~~~text
Debug:
PathOwner.Rebuild()
FloorOwner.Rebuild()
NavOwner.Rebuild()
~~~

Debug 코드가 내부 ownership 경계를 우회하면 실제 public contract 검증이 되지 않는다.

## 18. 초기 구현 범위

### 지금 구현

- GenerationSettings Snapshot
- Space Owner handle을 받는 GenerateBattle
- BattleReady ownership
- 전체 생성
- 안전한 Full Rebuild
- 현재 실제 설정 그룹의 semantic Apply...Settings entry point
- 필요한 feature-specific partial rebuild
- 각 Feature Owner의 resource ownership
- Debug/Test를 통한 rebuild 검증
- package profiling

### 지금 구현하지 않음

- generic DirtyFlags graph
- automatic dependency planner
- Requested/Building/Committed Revision framework
- cancellation/supersede framework
- generic Frame Budget Scheduler
- generic readiness aggregator
- worker-thread framework

## 19. 단계별 도입 조건

### Tier 1 - 현재

의미 기반 public API와 ownership을 완성한다.

~~~text
Caller
-> LevelManager semantic request
-> LevelManager policy
-> Feature Owner implementation
~~~

### Tier 2 - 반복되는 dependency가 실제로 늘어나면

- DirtyFlags
- dependency table/planner
- finer local/region rebuild
- readiness aggregation

이때도 public caller는 의미만 전달한다.

### Tier 3 - 실제 동시성/성능 문제가 확인되면

- Revision
- cancellation/supersede
- generic job scheduler
- frame budget
- worker threads
- advanced progress

이때도 Feature Owner resource ownership은 유지한다.

## 20. Acceptance Criteria

### Public API

- 외부 호출자는 의미 있는 설정 변경만 전달한다.
- 외부 호출자는 Rebuild 함수 조합을 하지 않는다.
- 외부 호출자는 BattleReady를 직접 변경하지 않는다.

### Ownership

- LevelManager가 Battle generation/rebuild 정책과 순서를 닫는다.
- 각 Feature Owner가 자기 구현과 runtime resource를 닫는다.
- 모든 Feature를 Manager 객체로 강제하지 않는다.
- TargetSpace는 02의 Space Owner handle이다.

### Rebuild

- Full Rebuild 가능
- 현재 실제 설정의 partial rebuild 가능
- dependency는 LevelManager 내부에서 닫힘
- rebuild resource가 중복/누수 없이 교체됨

### Debuggability

- Path 문제는 Path owner / LevelManager path policy 경로를 보면 된다.
- Floor resource 문제는 Floor owner를 보면 된다.
- transition readiness 문제는 SpaceTransition을 보면 된다.
- Space reference/lifecycle 문제는 Space Owner를 보면 된다.

### Performance

- 실제 병목을 먼저 측정한다.
- 병목 기능의 owner에서 먼저 해결한다.
- 공통 scheduler는 공통 문제가 확인된 뒤 추가한다.

## 21. Deferred

- player-facing Map Settings UI
- generic Dirty dependency graph
- automatic partial rebuild planner
- Revision system
- cancellation/supersede
- generic Frame Budget Scheduler
- generic readiness aggregator
- advanced worker-thread parallelism
- exact progress UI/VFX
- mid-battle live map mutation
- multi-revision cache

## 22. 핵심 요약

~~~text
Caller
  -> "Path settings changed"

LevelManager
  -> owns rebuild policy
  -> BattleReady false
  -> calls required Feature Owners
  -> validates
  -> BattleReady true

Feature Owner
  -> owns its calculation
  -> owns its data
  -> owns its runtime resources
~~~

미래 framework를 미리 만드는 것이 목표가 아니다.

미래 기능이 추가되어도 public 호출 경로와 ownership을 뒤집지 않아도 되는 구조를 지금 만든다.
