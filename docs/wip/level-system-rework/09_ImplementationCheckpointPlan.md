# 09. Level System Implementation Checkpoint Plan

> Status: Next implementation roadmap
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. 목적

현재 레벨 시스템 설계는 `MapGeneration -> Area -> Connection -> Path -> Floor -> Environment -> PCG`의 책임과 주요 알고리즘이 이미 정리되어 있다.

다음 구현에서는 여러 기능을 한 번에 완성하려 하지 않고, 각 단계가 독립적으로 실행/검증 가능한 체크포인트가 되도록 순차 구현한다.

핵심 목표:

- 한 번에 한 책임만 구현한다.
- 각 체크포인트는 그 자체로 확인 가능한 결과를 남긴다.
- 다음 체크포인트가 실패해도 이전 결과는 그대로 유효해야 한다.
- 구현 중 문제가 생기면 해당 기능 이름을 따라 한곳만 보면 되게 한다.
- RVT를 포함한 지면 표현 기술을 아직 확정하지 않아도 물리/공간 생성은 계속 진행한다.

## 2. 현재 RVT 결정

RVT(Runtime Virtual Texture)는 현재 확정 구현 기술이 아니다.

RVT 여부와 무관하게 다음 데이터/물리 구조는 동일하게 유지한다.

```text
MapSeed
-> Sites
-> Voronoi
-> Area
-> Adjacency
-> Connection
-> Gate
-> Path
-> Floor Geometry
-> Terrain Geometry
-> Collision
-> Navigation / DropLink
```

RVT는 위 결과를 소비하는 표현 계층 후보로만 취급한다.

따라서 다음은 RVT에 의존하지 않는다.

- Area/Cell topology
- Environment ownership data
- Connection/Path
- Dynamic Mesh floor/terrain
- Collision
- NavMesh/DropLink
- gameplay 판정

RVT 검토는 Ground Material, Environment Blend, Path visual expression, PCG object-ground blending을 실제로 연결할 시점에 별도 Probe로 수행한다.

## 3. 체크포인트 규칙

각 CP 구현 시 다음 원칙을 지킨다.

1. 현재 CP 완료 조건에 필요하지 않은 다음 기능을 선행 구현하지 않는다.
2. 디버그 시각화 또는 명확한 검증 결과가 있어야 완료로 본다.
3. MapSeed reroll을 실패 복구 수단으로 사용하지 않는다.
4. 기존 문서의 책임 경계를 바꾸는 문제가 발견되면 임의 우회 구현 대신 설계를 다시 검토한다.
5. CP가 끝날 때 다음 CP가 소비할 최소 데이터만 공개한다.
6. 새 Manager/Registry/Context/범용 Framework는 실제 필요가 확인되지 않으면 추가하지 않는다.

---

## CP1. Deterministic Sites

### 구현

```text
MapSeed
-> N x N Logical Slots
-> deterministic jittered Site
```

### 완료 조건

- 같은 Seed와 Settings에서 같은 Site 결과가 나온다.
- 각 Logical Slot에 정확히 하나의 Site가 존재한다.
- Jitter 결과가 자신의 허용 영역을 벗어나지 않는다.
- Site와 Logical Slot을 Debug Draw로 한눈에 확인할 수 있다.
- 다수 Seed에서 생성 실패가 없다.

### 제외

- Voronoi
- Area 역할
- Connection
- Floor

---

## CP2. Bounded Voronoi Cells

### 구현

```text
Sites
-> bounded Voronoi
-> Cell polygon
-> Cell centroid / area
```

### 완료 조건

- MapBounds 안에 Cell polygon이 생성된다.
- 각 Site와 Cell의 대응이 명확하다.
- Polygon winding/기본 validity가 정규화된다.
- Cell 경계를 Debug Draw할 수 있다.
- Voronoi 표현이 실패하는 Seed는 기존 설계의 regular logical-slot fallback으로 유효 결과를 만든다.
- Seed reroll은 없다.

### 중요

이 단계가 첫 번째 geometry validity checkpoint다.

---

## CP3. Area Classification + True Adjacency

### 구현

```text
Cell
-> Gameplay / Auxiliary classification
-> true shared-boundary adjacency
```

### 완료 조건

- `MinCombatCellArea` 기준으로 Gameplay/Auxiliary가 결정된다.
- 점 하나만 닿는 Cell은 이웃으로 취급하지 않는다.
- 실제 non-zero shared boundary가 있는 Cell만 adjacency를 가진다.
- Area 역할과 adjacency를 Debug Draw할 수 있다.
- 모든 adjacency는 양방향으로 일관된다.

### 제외

- 실제 traversal connection 선택
- Gate
- Path

---

## CP4. Traversal Connection Graph

### 구현

```text
Area adjacency graph
-> required Gameplay Area connectivity
-> backbone
-> minimum necessary Auxiliary transit
-> optional extra connection
```

### 완료 조건

- 모든 required Gameplay Area가 하나의 traversal graph로 연결된다.
- Auxiliary를 사용하지 않고 연결 가능하면 불필요한 Auxiliary transit을 만들지 않는다.
- Auxiliary가 필요한 경우 최소한의 transit 경로를 사용할 수 있다.
- 동일 Seed는 동일 Connection 결과를 만든다.
- Connection graph를 Debug Draw할 수 있다.
- batch Seed 검증에서 disconnected required Area가 없다.

### 결과

이 CP가 끝나면 맵의 논리적 이동 구조가 성립한다.

---

## CP5. Gate + Path

### 구현

```text
Connection
-> shared-boundary Gate
-> Path centerline
-> corridor polygon
```

초기 Path는 기존 설계대로 단순 경로를 우선한다.

```text
Centroid(A)
-> GateCenter
-> Centroid(B)
```

### 완료 조건

- 모든 Connection에 유효한 shared-boundary Gate가 있다.
- 모든 Gate에 연결되는 Path centerline이 있다.
- corridor polygon이 생성된다.
- 여러 Path의 접합은 별도 Junction 객체 없이 polygon union/공간 결과로 처리한다.
- Gate, centerline, corridor를 각각 Debug Draw할 수 있다.

### 제외

- Floor mesh
- Cut/Tunnel
- PCG road visual

---

## CP6. Basic Physical Floor

### 구현

```text
Area + Required Path
-> flat Ground Dynamic Mesh
-> Collision
```

이 단계에서는 terrain variation을 만들지 않는다.

### 완료 조건

- 최종 Ground polygon에서 실제 Dynamic Mesh가 생성된다.
- required Path가 물리적으로 끊기지 않는다.
- Player가 생성된 Floor 위를 실제로 이동할 수 있다.
- Collision은 mesh batch 완료 후 갱신한다.
- invalid triangle index나 필수 바닥 누락이 없다.
- mesh build/collision cook 시간을 확인할 수 있다.

### 결과

이 CP가 끝나면 절차 생성된 맵에서 실제 Character 이동이 가능하다.

---

## CP7. Terrain Geometry

### 구현 순서

```text
Ground
-> HighGround
-> Path through non-ground
-> Cut
-> conservative Tunnel
-> Pit / Blocked expression
```

### 완료 조건

- HighGround top과 cliff wall이 생성된다.
- required Path는 TerrainRole보다 우선한다.
- HighGround를 통과하는 Path는 기존 fallback hierarchy를 따른다.

```text
Tunnel invalid
-> Cut

Cut invalid
-> Ground
```

- Pit/Blocked를 통과해야 하는 required Path에는 base-level traversal floor가 유지된다.
- Tunnel entry/exit가 wall로 막히지 않는다.
- fallback 결과가 최종 TerrainRole/mesh와 일치한다.
- Terrain별 Debug Draw 또는 명확한 테스트 시나리오가 있다.

### 중요

처음부터 복잡한 3D boolean terrain을 만들지 않는다.

---

## CP8. Navigation + DropLink

### 선행 조건

Floor와 Collision이 최종 상태여야 한다.

### 구현

```text
Final Floor Collision
-> Runtime NavMesh
-> lower/upper navigation validation
-> Drop candidate
-> one-way DropLink
```

### 완료 조건

- 일반 Ground에서 Monster NavMesh가 생성된다.
- HighGround top에도 필요한 NavMesh가 생성된다.
- lower/upper surface가 의도한 대로 분리/유효하다.
- Drop source와 landing point가 실제 Nav/physical floor 기준으로 검증된다.
- upward traversal을 요구하는 link를 만들지 않는다.
- 필요한 HighGround에만 one-way DropLink를 만든다.
- AI가 실제 link를 사용하는 동작은 AI owner의 책임으로 유지한다.

### 결과

여기까지가 RVT 결정과 무관한 핵심 공통 구현부다.

---

## 4. RVT / 표현 기술 결정 지점

CP8 완료 후 실제 Ground/Path/Terrain과 PCG 표현을 붙이기 전에 RVT Probe를 수행할 수 있다.

검토 대상:

- Dynamic Mesh Ground Material과 RVT 출력/샘플링 궁합
- Environment Boundary Blend 표현
- Path와 주변 Ground의 시각적 전환
- PCG rock/vegetation과 지면 blending
- Runtime local Environment 변경 시 갱신 범위/비용

### 판단 기준

RVT를 쓰더라도 다음 구조는 바꾸지 않는다.

```text
Area / Environment / Path / Terrain
= authoritative spatial/gameplay data

Material / RVT / PCG
= presentation consumers
```

RVT 없이도 요구 품질을 만족한다면 사용하지 않아도 된다.

RVT가 실제 표현 품질/작업 비용에 이점이 있을 때만 채택한다.

---

## 5. 공통 구현 이후 체크포인트

공통 구현부 이후 세부 문서는 별도로 확정한다.

예상 흐름:

```text
CP9  Environment assignment / Boundary Blend data
CP10 Ground Material + optional RVT integration
CP11 PCG handoff / environment expression
CP12 Battle Space + LevelManager / BattleReady integration
CP13 Seed Sweep / Multiplayer validation / Cleanup
```

정확한 순서는 CP8 결과와 실제 구현 상태를 보고 다시 조정할 수 있다.

특히 Material/RVT와 Environment/PCG의 세부 순서는 표현 Probe 결과가 나오기 전까지 architecture contract로 고정하지 않는다.

## 6. Codex 작업 방식

Codex에는 한 번에 하나의 CP만 구현하도록 지시한다.

예:

```text
Implement CP4 only.
Do not begin Gate, Path, Floor, Terrain, Navigation, Environment or PCG work.
Finish CP4 validation/debug output and stop.
```

각 CP가 끝난 뒤 실제 코드와 결과를 검수하고 다음 CP로 넘어간다.

문제가 발견되면 이전 CP까지 되돌아가 구조를 넓게 다시 짜기보다, 문제가 발생한 Feature Owner와 해당 CP의 데이터 계약부터 확인한다.

## 7. 전체 완료 기준

레벨 생성 본체의 최소 완료 상태는 다음을 만족한다.

```text
Seed
-> valid topology
-> guaranteed traversal
-> physical Floor/Terrain
-> Collision
-> Monster Navigation
-> Environment expression
-> PCG decoration
-> BattleReady
```

그리고:

- 모든 MapSeed는 usable map을 만든다.
- reroll에 의존하지 않는다.
- multiplayer에서 authoritative gameplay 결과가 일관된다.
- presentation 실패가 topology/collision/navigation을 무효화하지 않는다.
- 각 단계의 문제는 해당 기능 책임자를 따라 추적할 수 있다.
