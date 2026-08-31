# 02 - Shared Trait Tree

## 1. 핵심 개념

### Decision: Trait Tree는 Monster가 아니라 변주 축에 소속된다

예:

- Shared Weapon Trait Tree
- Shared Body Trait Tree
- Shared Element Trait Tree
- Shared Armor Trait Tree
- Shared Mutation Trait Tree

각 Tree는 여러 Monster Set이 공유할 수 있다.

```
Goblin Set
├ Weapon Tree
├ Body Tree
├ Element Tree
└ Armor Tree

Slime Set
├ Body Tree
├ Element Tree
└ Mutation Tree
```

Tree 사용 여부를 Humanoid/Beast/Generic 같은 상위 Type Preset이 자동으로 정하지 않는다.

### Decision: Monster Set이 Tree를 직접 선택한다

휴머노이드라고 반드시 Weapon Tree를 사용할 필요가 없고, 비휴머노이드라고 반드시 Weapon Tree를 사용할 수 없는 것도 아니다.

따라서 Monster Set에 직접 Tree 참조 목록을 둔다.

이 결정은 중간 Preset 계층을 제거하고 예외 규칙을 줄인다.

## 2. Tree 구조

### Decision: 각 Trait Tree는 단일 부모 계층이다

Tree 내부에서 하나의 Node는 최대 하나의 Parent를 가진다.

예:

```
Weapon
├ Sword
│  ├ Speed Sword
│  │  ├ Extreme Speed Sword
│  │  └ Trick Sword
│  └ Strong Sword
└ Spear
   ├ Speed Spear
   └ Heavy Spear
```

다중 부모를 허용하지 않는다.

```
A ─┐
   ├→ C
B ─┘
```

같은 형태는 초기 설계 범위에서 금지한다.

여러 축의 조합은 Tree 내부 다중 상속이 아니라 여러 Shared Trait Tree를 동시에 적용하는 방식으로 해결한다.

## 3. 경로 누적

### Decision: 선택 Node만 적용하지 않고 Root부터 경로 전체를 누적한다

예:

```
Sword
→ Speed Sword
→ Extreme Speed Sword
```

최종 결과:

```
Sword Delta
+ Speed Sword Delta
+ Extreme Speed Sword Delta
```

각 단계는 이전 단계의 의미를 보존하면서 추가 변화를 제공한다.

이 구조는 "상속"과 비슷하지만 C++ Class 상속이 아니라 데이터 누적이다.

## 4. 다중 Tree 합성

### Decision: Final Monster는 모든 선택 Tree의 결과를 합친다

예:

```
Base: Goblin

Weapon:
Sword → Speed Sword → Extreme Speed Sword

Body:
Normal → Large

Element:
Neutral → Fire
```

Resolved 결과:

```
Goblin Base
+ Sword
+ Speed Sword
+ Extreme Speed Sword
+ Large
+ Fire
```

여기서 Weapon/Body/Element는 서로 다른 독립 축이다.

## 5. Trait Node가 변경하는 범위

### Decision: 초기 범위는 Skill / Stat / Visual

초기 Trait Node는 최소한 다음 Delta를 표현한다.

- Skill Delta
- Stat Delta
- Visual Delta

### Skill Delta

목적은 기존 MA Skill System의 구성을 변경하는 것이다.

정확한 Add/Remove/Replace API는 구현 전 현재 Skill Module/Slot 구조와 맞춰 결정한다.

초기 개념 예:

```
Sword
- Basic Attack을 Sword 계열로 구성
- Sword 관련 Skill 추가

Speed Sword
- 빠른 공격/연계 Module 추가
- Heavy 계열 일부 제거 또는 가중치 감소

Fire
- 공격에 Fire 관련 Module/Effect 추가
```

### Stat Delta

예:

- MaxHP
- MoveSpeed
- AttackRange
- AreaScale
- Knockback 관련 값

정확한 적용 방식은 기존 Attribute/GameplayEffect 계층을 우선 재사용한다.

### Visual Delta

예:

- Weapon Mesh
- Armor Mesh
- Material
- VFX
- Scale/Silhouette

Visual은 단순 장식이 아니라 Gameplay Telegraph 역할을 가진다.

### Deferred: Behavior Override

초기 Trait Node가 StateTree/BT 자체를 직접 바꾸게 하지 않는다.

먼저 Skill/Stat/Visual 합성만으로 충분한지 검증한다.

정말 행동 구조 자체가 바뀌는 Trait 요구가 확인되면 별도 확장을 검토한다.

## 6. Complexity

### Candidate: Node별 Complexity Cost

Tree Depth를 그대로 난이도로 사용하지 않는다.

같은 Depth라도 실제 전투 영향이 다를 수 있기 때문이다.

예:

```
Sword               Cost 1
Speed Sword         Cost 1
Extreme Speed Sword Cost 3
Fire                Cost 2
Large               Cost 2
```

몬스터 또는 Encounter에서 주어진 Complexity Budget 범위 안에서 Trait 경로를 구성한다.

### 목적

Complexity는 단순 스탯 강화를 넘어서 다음을 제어한다.

- 적용되는 Trait의 수
- 한 Tree에서 도달하는 깊이
- Skill Set의 복잡성
- 시각적 변주 정도
- 전투 판단에 필요한 선택지 수

### Experiment: Encounter Cost와의 관계

`Complexity`와 기존 Monster Spawn/Encounter Cost를 동일 값으로 만들지 않는다.

어떤 Trait은 전투 난이도를 크게 올리지만, 어떤 Trait은 행동 다양성만 늘릴 수 있다.

초기에는 별도 값으로 유지하고 플레이 테스트 후 관계를 결정한다.

## 7. 랜덤 생성 규칙

### Decision: Skill을 직접 랜덤 지급하지 않는다

기본 랜덤화 대상은 Trait다.

```
Monster Set
→ 사용할 Trait Tree 목록
→ Complexity Budget
→ 각 Tree에서 유효 경로 선택
→ Node Delta 누적
→ Resolved Monster
```

이 방식의 목적:

- 랜덤 결과에도 구조적 의미가 있다.
- 외형과 Skill 변화가 연결될 수 있다.
- 같은 Trait를 여러 종에서 재사용할 수 있다.
- 스킬 조합을 디자이너가 추적하기 쉽다.

### Open Question: Budget 분배 정책

다음은 아직 확정하지 않는다.

- 모든 Tree가 최소 1단계 선택되어야 하는가
- 일부 Tree는 선택하지 않아도 되는가
- 특정 Tree에 Budget 가중치를 둘 것인가
- 깊은 한 축 vs 얕은 여러 축의 선택 확률

초기 샘플 구현에서 분포를 확인한 뒤 결정한다.

## 8. Tree 재사용 예

공용 Weapon Tree:

```
Sword
├ Speed Sword
│  ├ Extreme Speed Sword
│  └ Trick Sword
└ Strong Sword
```

Goblin:

```
Goblin Base
+ Sword
+ Speed Sword
```

Golem:

```
Golem Base
+ Sword
+ Strong Sword
```

같은 Tree를 사용하지만 Base Monster의 Stat/Visual/기본 Skill이 다르므로 최종 결과는 서로 다르다.

## 9. 데이터/에디터 표현

### Candidate: Tree 하나 = Asset 하나

피해야 할 구조:

```
DA_Sword
DA_SpeedSword
DA_ExtremeSpeedSword
DA_StrongSword
...
```

권장 방향:

```
TT_Weapon
└ Nodes[]
```

Tree 하나의 Asset 내부에 여러 Node를 저장한다.

### Candidate: 전용 Node Graph Editor

Trait 제작자는 DataTable row를 직접 관리하기보다 다음 UX를 사용하는 것이 목표다.

```
[Sword] ─→ [Speed Sword] ─→ [Extreme Speed Sword]
   └────→ [Strong Sword]
```

Node 선택 시 Details에서 Cost와 Skill/Stat/Visual Delta를 편집한다.

UE Editor 구현 후보:

- `UEdGraph`
- `UEdGraphNode`
- `UEdGraphSchema`
- custom asset editor toolkit

### Decision: Editor Graph와 Runtime Data를 분리한다

에디터 그래프는 제작 UI다.

게임 런타임은 GraphEditor/Slate 의존 없이 평탄화된 데이터만 읽어야 한다.

개념:

```
Editor Graph
→ validate/compile
→ compact runtime nodes
→ resolver
→ Resolved Monster
```

### Experiment: Graph Editor 구현 시점

전용 Graph Editor의 방향은 유력하지만, 첫 구현 순서에서는 Runtime Data Model과 Resolver를 먼저 검증한다.

이유:

- 데이터 모델이 바뀌면 에디터 구현 비용이 그대로 재작업된다.
- 2~3개 실제 Tree로 구조가 충분한지 먼저 확인할 수 있다.

데이터 모델이 확정되면 Graph Editor를 핵심 제작 툴로 승격한다.

## 10. 금지해야 할 초기 과설계

다음은 실제 요구가 생기기 전 만들지 않는다.

- Trait 간 임의 다중 상속
- 모든 몬스터용 Capability 시스템
- Humanoid/Beast 타입별 자동 Tree Preset
- 모든 Trait 조합별 Synergy Table
- 런타임마다 Parent Chain을 반복 탐색하는 구조
- Node별 개별 DataAsset 생성

Resolver는 생성/초기화 시 최종 결과를 만들고, 전투 중에는 Resolved 결과를 사용한다.
