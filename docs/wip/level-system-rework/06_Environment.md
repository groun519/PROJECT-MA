# 06. Environment

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. 목적

Environment 시스템은 각 Area가 어떤 환경 표현을 사용하는지 결정하고, 서로 다른 환경이 맞닿는 경계만 제한된 폭으로 블렌딩한다.

핵심 분리:

> Area = 명확한 Environment 소유권.
> Environment Profile = 해당 환경의 PCG/지면/표현 의미.
> Blend Boundary = 서로 다른 Environment가 만나는 좁은 전환 구간.

Environment는 직접 나무/바위/장식물을 Spawn하지 않는다. 실제 표현은 PCG/Material 계층이 Environment 데이터를 소비해서 만든다.

## 2. 확정 방향

### Decision

전역 Anchor 기반 Spatial Weight Field는 초기 구현에 사용하지 않는다.

대신:

1. Environment별 Source Area를 MapSeed 기반으로 선택한다.
2. 모든 Source에서 동시에 Multi-Source Dijkstra를 수행한다.
3. 각 Area는 먼저 도달한 Environment의 소유권을 얻는다.
4. Area 내부는 기본적으로 하나의 명확한 Environment를 가진다.
5. 서로 다른 Environment가 맞닿는 공유 Boundary 주변에서만 좁게 블렌딩한다.

이 구조는 단일 Environment 맵과 다중 Environment 맵을 같은 시스템으로 처리한다.

단일 Environment만 사용하는 경우 모든 Area가 동일 Environment를 가지며 Blend Boundary는 생성되지 않는다.

## 3. 왜 전역 Weight Field를 사용하지 않는가

넓은 Spatial Weight Field는 자연스러운 혼합에는 유리하지만 Runtime 지형/환경 변경과 결합할 때 변경 영향 범위가 불필요하게 커질 수 있다.

예를 들어 특정 Area가 Runtime에 Forest -> Corruption으로 즉시 변하는 기믹이 생겼을 때, 넓은 Weight Field가 주변 넓은 지역까지 애매하게 섞이게 만들면 변화의 공간적 의미가 흐려진다.

현재 목표는:

- Area 내부 환경 정체성은 명확하게 유지;
- 경계는 시각적으로 칼같이 끊기지 않을 정도만 보간;
- Runtime 변경 시 변경된 Area와 직접 인접한 경계만 갱신 가능;
- PCG 재생성 범위를 지역적으로 유지.

따라서 Environment 분포와 Blend를 분리한다.

## 4. Environment Profile

각 Area는 최소한 하나의 `EnvironmentID` 또는 동등한 Profile 참조를 가진다.

Environment Profile이 표현 계층에 제공할 수 있는 의미 예:

- PCG profile/graph selection;
- Ground material profile;
- vegetation set;
- rock/decor set;
- ambient/VFX/sound profile;
- future gameplay/environment tags.

### Non-requirement

현재 단계에서 Environment마다 반드시 하나의 거대한 PCG Graph를 가져야 한다고 규정하지 않는다.

Environment 시스템은 환경 의미/Profile만 제공한다. 실제로 하나의 Graph로 구성할지 여러 Graph/Settings로 나눌지는 PCG 구현 단계에서 결정한다.

## 5. Source Area 선택

### Decision

Environment 분포는 발원지(Source Area)에서 시작한다.

설정은 Environment별 `SourceCount`를 지정할 수 있어야 한다.

예:

- Forest Source x3
- Desert Source x1

Source 개수와 Environment 종류 개수는 같을 필요가 없다.

### Source selection rule

초기 구현:

- MapSeed에서 파생된 deterministic random stream 사용;
- 유효한 Area 중 Source Area 선택;
- 동일 Area를 두 Source가 동시에 점유하는 중복만 금지;
- Source끼리 최소 거리 조건은 두지 않는다;
- 같은 Environment가 여러 Source를 가져도 된다.

Source 간 최소 거리, 외곽 선호, 특정 TerrainRole 선호 같은 규칙은 실제 결과가 필요하다고 증명할 때 추가한다.

## 6. Environment 확산

### Decision

Area adjacency graph 위에서 Multi-Source Dijkstra를 사용한다.

모든 Source를 동시에 초기 frontier에 넣고, 각 Source가 자신의 Environment 소유권을 인접 Area로 확장한다.

### Edge cost

초기 비용:

`Cost(A -> B) = Distance(Centroid(A), Centroid(B))`

단순 BFS처럼 Area 하나를 항상 비용 1로 취급하지 않는다.

이유:

- Voronoi Area 크기가 서로 다를 수 있음;
- 실제 공간적 거리를 어느 정도 반영할 수 있음;
- 기존 Area centroid 데이터를 그대로 사용할 수 있음.

### Tie-break

동일 누적 비용 또는 수치 허용오차 내 동률이 발생하면 MapSeed에서 파생된 deterministic tie-break를 사용한다.

동일 Seed/설정은 항상 동일 Environment 분포를 만들어야 한다.

## 7. 할당 결과

Dijkstra 완료 후 모든 유효 Area는 정확히 하나의 dominant `EnvironmentID`를 가진다.

환경 소유권은 Area 단위다.

따라서 하나의 Area 안에서 Forest 0.6 / Desert 0.4 같은 전역 Weight를 기본 데이터로 저장하지 않는다.

필요한 연속 Weight는 오직 이종 Environment Boundary의 Blend Band 안에서만 파생한다.

## 8. Boundary Blend

### Decision

Blend는 서로 다른 Environment를 가진 인접 Area 사이의 실제 공유 Boundary에서만 생성한다.

같은 Environment끼리 맞닿은 Boundary에는 별도 Blend가 필요 없다.

초기 파라미터:

- `BlendWidth`

경계에서 양쪽 Environment Weight가 제한된 거리 동안 보간된다.

개념 예:

- Forest 내부: Forest 1 / Desert 0
- 경계 접근: Forest 0.75 / Desert 0.25
- Boundary 부근: Forest 0.5 / Desert 0.5
- Desert 쪽: Forest 0.25 / Desert 0.75
- Desert 내부: Forest 0 / Desert 1

### 중요한 원칙

Blend Band는 넓은 환경 생성 영역이 아니라 시각적/PCG 전환을 위한 좁은 경계 보정이다.

`BlendWidth`의 정확한 수치는 콘텐츠 튜닝 값이며 현재 설계에서 고정하지 않는다.

## 9. Blend 소비

같은 Boundary Blend 의미를 Ground Material과 PCG가 공유할 수 있어야 한다.

그러나 각 소비자가 Weight를 반드시 같은 방식으로 사용할 필요는 없다.

예:

- Ground material: 직접 선형/곡선 blend;
- Grass density: weight curve;
- Tree spawn: threshold 또는 curve;
- Rock variant: weighted selection.

따라서 Environment 시스템은 공통 Blend Weight/거리 정보를 제공하고, 구체적 표현 Curve는 소비자가 소유한다.

## 10. Area와 PCG의 관계

Area는 자신의 Environment 의미를 가진다.

PCG는 최종 Area polygon과 Environment 정보를 입력으로 받아 해당 환경 표현을 생성한다.

예:

`Area -> Environment Profile -> PCG expression`

중요:

- PCG 결과를 역으로 읽어 Area Environment를 결정하지 않는다;
- Environment 분포는 PCG보다 먼저 결정된다;
- Gameplay와 PCG가 필요하면 동일 Environment 데이터를 소비한다.

## 11. Runtime Environment 변경

### Decision

특정 Area의 Environment가 Runtime에 직접 변경될 수 있는 구조를 막지 않는다.

예:

`Forest -> Corruption`

기본 변경 범위:

1. 해당 Area의 `EnvironmentID` 변경;
2. 해당 Area 표현 재생성/갱신;
3. 해당 Area와 직접 인접한 Area들의 Boundary Blend 재평가;
4. 필요한 경우 해당 local PCG 영역만 dirty/rebuild.

### Non-requirement

단일 Area 변경이 자동으로 주변 Area까지 Environment를 재확산시키는 기능은 현재 구현하지 않는다.

오염 확산, 정화 전파처럼 주변 소유권 자체가 변하는 기믹이 실제로 추가될 때 별도의 gameplay propagation 규칙으로 설계한다.

초기 Source/Dijkstra 생성 알고리즘과 Runtime propagation mechanic을 같은 책임으로 묶지 않는다.

## 12. Terrain/Path와의 관계

Environment 소유권은 Area의 TerrainRole이나 Path 존재와 별개다.

예:

- Forest + Ground
- Forest + HighGround
- Desert + Pit
- Corruption + TransitRequired

Path가 Environment 경계를 통과할 경우 Path 표현도 해당 위치의 Boundary Blend 정보를 소비할 수 있다.

Environment 때문에 required Path의 topology나 traversability를 변경하지 않는다.

## 13. Gameplay와의 관계

Environment 데이터는 미래에 다음 시스템의 입력이 될 수 있다.

- monster pool/weight;
- resource tendency;
- event selection;
- ambient sound/VFX.

하지만 현재 Environment 구현의 핵심 책임은 spatial environment assignment와 boundary transition이다.

Gameplay 규칙을 Environment 문서에서 미리 확장하지 않는다.

## 14. Multiplayer

서버가 authoritative Environment 분포와 Runtime Environment 변경을 소유한다.

초기 생성이 완전히 결정적이라면 동일 MapSeed와 설정으로 클라이언트가 동일 분포를 재구성할 수 있다.

Gameplay에 영향을 주는 Environment 판정은 서버의 authoritative Area Environment 데이터를 기준으로 한다.

Runtime 변경은 필요한 EnvironmentID/변경 이벤트만 동기화하고 PCG 인스턴스 전체를 복제하지 않는다.

## 15. Recommended data

기존 프로젝트의 동등한 타입을 우선 재사용한다.

Environment 설정 최소 의미:

- `EnvironmentID` / Profile reference
- `SourceCount`
- default `BlendWidth` 또는 Environment pair가 소비할 수 있는 equivalent setting

Area 최소 추가 의미:

- assigned `EnvironmentID`

생성 과정/debug용 데이터:

- selected Source AreaIDs
- winning Source/Environment
- accumulated Dijkstra cost

Runtime에 필요하지 않은 Dijkstra frontier/cache는 생성 후 유지할 필요가 없다.

## 16. Validation

생성 후:

- 모든 configured Environment의 SourceCount가 유효 범위인지 확인;
- Source Area 중복 없음;
- 모든 유효 Area가 정확히 하나의 Environment를 가짐;
- 모든 Environment assignment가 유효한 configured Environment를 참조;
- 동일 Seed/설정에서 동일 Source와 동일 최종 분포;
- Blend Boundary는 실제 인접 Area 사이에서만 존재;
- 같은 Environment 사이에는 불필요한 Blend Boundary 없음;
- `BlendWidth >= 0`;
- Environment 변경 후 변경 Area와 인접 Boundary 상태가 일관됨.

SourceCount 총합이 Area 수를 초과하는 잘못된 설정은 validation/configuration error로 처리한다. MapSeed reroll로 해결하지 않는다.

## 17. Debugging

필수 디버그 표현:

- Area별 Environment color;
- Source Area 강조;
- Source별 Dijkstra 확산 결과/누적 cost 선택 표시;
- 서로 다른 Environment Boundary 표시;
- Blend Band 폭 표시;
- Runtime 변경 시 dirty Area/Boundary 표시.

Environment 분포는 한눈에 확인 가능해야 한다.

## 18. 현재 제외한 대안

### Global Anchor Spatial Weight Field

현재 제외 이유:

- Area별 명확한 환경 정체성이 약해짐;
- Runtime 국소 변경 시 영향 범위가 넓어질 수 있음;
- 초기 요구보다 Weight 계산/캐싱 책임이 커짐.

필요성이 확인되면 특정 기믹/표현 전용 보조 Field로 다시 검토할 수 있지만 Environment 소유권의 기본 모델로 사용하지 않는다.

### Pure random Area assignment

현재 제외 이유:

- Environment가 체스판처럼 조각날 가능성이 큼;
- 환경 덩어리의 공간적 연속성이 약함.

### Multi-Source BFS with unit cost

현재 제외 이유:

- 크기가 다른 Voronoi Area를 모두 동일한 한 칸으로 취급함;
- centroid distance 기반 Dijkstra가 현재 데이터로 구현 가능하면서 실제 공간을 더 잘 반영함.

## 19. Codex implementation contract

Codex는 구현 전에 기존 Environment/PCG 관련 타입이 있는지 확인하고 동등한 구조를 재사용한다.

초기 구현 범위:

1. deterministic Source Area selection;
2. Multi-Source Dijkstra assignment;
3. Area EnvironmentID 저장;
4. 이종 Environment shared-boundary detection;
5. Blend Band를 소비자가 샘플할 수 있는 최소 데이터/API;
6. deterministic tests/debug visualization.

다음은 현재 구현 범위가 아니다:

- 전역 Anchor Weight Field;
- Noise 기반 대형 환경 분포;
- Runtime 자동 환경 확산;
- 복잡한 다중 Environment 동시 weight vector;
- PCG graph architecture 자체의 재설계.

## 20. Acceptance baseline

최소 seed sweep에서:

- generation이 Environment 단계 때문에 seed reroll을 요구하지 않음;
- 모든 Area가 Environment를 받음;
- 같은 seed/settings는 같은 Source와 같은 assignment 생성;
- 서로 다른 Environment는 연속된 Area cluster를 형성할 수 있음;
- Blend는 이종 경계 주변으로 제한됨;
- 단일 Environment 설정에서는 Blend Boundary가 0개임.

Runtime Environment 변경 기능을 구현하는 시점에는 추가로:

- 한 Area 변경이 비인접 Area의 Environment 소유권을 암묵적으로 바꾸지 않음;
- dirty/rebuild 범위가 변경 Area와 필요한 인접 경계로 제한되는지 검증한다.
