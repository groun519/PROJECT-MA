# 05 - Implementation Plan

## 1. 구현 전략

이 문서의 목표는 한 번에 전체 Monster Rework를 완성하는 것이 아니다.

핵심 데이터 모델을 먼저 검증하고, 가장 재작업 비용이 큰 전용 Editor Tool은 데이터 모델이 살아남은 뒤 만든다.

순서:

```
Current code audit
→ Trait runtime model
→ Resolver
→ Skill/Stat/Visual integration
→ Personality
→ AI/StateTree/EQS integration
→ Trait Graph Editor
→ tuning/content expansion
```

Spawn은 현재 로컬에서 해결되었으므로 이 순서에 포함하지 않는다.

## 2. Phase 0 - 구현 시작 전 동기화

### Requirement

실제 구현 Branch의 최신 로컬 변경이 repository에 반영된 상태에서 다시 Current Monster/Skill 코드를 확인한다.

특히 확인할 것:

- 로컬 Spawn Rework와 Monster Lifecycle
- `AMonster`
- `AMAAIController`
- Monster Skill Slot 데이터
- `UMASkillManagerComponent`
- Skill Module/Assembler 구조
- Pattern StateTree 관련 코드
- 기존 Monster Blueprint/Data authoring 경로

### Rule

현재 mockup 문서가 코드보다 우선한다고 가정하지 않는다.

코드가 이미 더 좋은 구조로 변경되었다면 기존 기능을 재사용한다.

## 3. Phase 1 - Trait Runtime Data Model

### 목표

UI 없이도 Shared Trait Tree의 데이터 의미를 표현할 수 있어야 한다.

### 최소 필요 개념

- Trait Tree Asset
- Trait Node
- Parent 관계
- Complexity Cost
- Skill Delta
- Stat Delta
- Visual Delta

### Requirement

한 Tree 안에서:

- Root/child 관계 검증
- Parent 최대 1개
- cycle 금지
- invalid parent 금지
- 안정적인 Node identity

가 가능해야 한다.

### Important

Node별 개별 DataAsset을 만들지 않는다.

```
TT_Weapon
└ Nodes[]
```

형태를 우선한다.

### Acceptance

코드/간단한 Details 데이터만으로 다음 Tree를 만들 수 있어야 한다.

```
Sword
├ Speed Sword
│  └ Extreme Speed Sword
└ Strong Sword
```

그리고 선택 경로 `Sword → Speed Sword → Extreme Speed Sword`를 resolve했을 때 세 Node의 Delta가 올바른 순서로 누적되어야 한다.

## 4. Phase 2 - Monster Set + Resolver

### 목표

Monster Set이 사용할 Shared Trait Tree를 직접 선택하고 여러 Tree 결과를 하나의 Resolved Monster로 합친다.

### 최소 흐름

```
Monster Set
→ Trait Tree references
→ selected path per tree
→ resolve parent chain
→ merge deltas
→ Resolved Monster configuration
```

### Requirement

Monster Type Preset/Capability 시스템을 추가하지 않는다.

Slime이 Weapon Tree를 쓰지 않는다면 Slime Set에서 Weapon Tree를 참조하지 않으면 끝이다.

### 샘플 검증

최소 3종:

- Goblin
- Slime
- Golem 또는 다른 Weapon 재사용 가능 Monster

최소 공용 Tree:

- Weapon
- Body
- Element

검증 포인트:

1. Goblin과 다른 Monster가 동일 Weapon Tree를 재사용할 수 있는가.
2. Slime은 Weapon Tree 없이 정상 동작하는가.
3. 여러 Tree의 결과가 충돌 없이 합쳐지는가.

## 5. Phase 3 - Skill Delta Integration

### 목표

Trait 결과가 실제 MA Skill System을 변경한다.

### Rule

새 Monster 전용 Skill System을 만들지 않는다.

기존 Skill Slot / Skill Module / Assembler / GAS 경로를 사용한다.

### Open implementation detail

Skill Delta의 정확한 연산은 현재 Skill System을 다시 확인한 뒤 결정한다.

후보:

- Add
- Remove
- Replace
- Override parameter/module

하지만 이 네 연산을 모두 선제 구현하지 않는다.

실제 샘플 Trait에 필요한 연산만 먼저 만든다.

### 샘플

Weapon:

```
Sword
→ Sword 기본 공격 구성

Speed Sword
→ 빠른 연계 Module 추가

Extreme Speed Sword
→ 추가 빠른 연계/이동 공격
```

Element:

```
Fire
→ 기존 공격에 Fire Module/Effect 추가
```

### Acceptance

같은 Goblin Base가 Trait 선택에 따라 실제 SkillManager에 서로 다른 최종 Skill 구성을 가져야 한다.

## 6. Phase 4 - Stat / Visual Integration

### Stat

기존 Attribute/GameplayEffect 적용 경로를 우선 재사용한다.

Trait용 별도 Stat 프레임워크를 만들지 않는다.

### Visual

최소 하나 이상의 Trait가 플레이어에게 외형으로 보이게 한다.

예:

- Sword → Sword Mesh
- Fire → Material/VFX
- Large → Scale/Silhouette

### Acceptance

플레이어가 Debug 정보 없이도 주요 Trait 하나 이상을 외형으로 구분할 수 있어야 한다.

## 7. Phase 5 - Complexity Selection

### 목표

Skill 직접 랜덤 지급 없이 Complexity Budget으로 Trait 변주를 만든다.

### 초기 알고리즘 요구

- 유효한 Tree path만 선택
- Budget 초과 금지
- Parent 누적 Cost 정확히 계산
- Seed를 사용할 경우 결과 재현 가능

### Experiment

깊은 단일 Tree와 여러 얕은 Tree 중 어느 쪽이 얼마나 자주 나오는지 통계를 확인한다.

최소 수백 회 생성 Debug 결과를 출력해 분포를 본다.

### 보류

Complexity와 Encounter Spawn Cost의 직접 환산 공식은 만들지 않는다.

## 8. Phase 6 - Individual Personality

### 목표

현재 확정 후보 7개 값을 데이터로 표현하고 실제 AI 판단 중 최소 일부에 연결한다.

값:

- Prediction
- Reaction
- Evasion
- Caution
- Initiative
- Persistence
- Preemptiveness

### 구현 순서 제안

전부 한 번에 연결하지 않는다.

첫 검증 우선순위:

1. Caution
2. Evasion
3. Prediction
4. Reaction
5. Preemptiveness
6. Persistence
7. Initiative

이 순서는 확정 Gameplay 우선순위가 아니라 구현 검증 난이도와 관찰 가능성을 기준으로 한 Candidate다.

### First acceptance examples

Caution:

- 낮은 몬스터는 사거리 끝에서 헛공격 가능
- 높은 몬스터는 더 접근/대기 후 공격

Evasion:

- 낮은 몬스터는 논타겟 공격을 거의 피하지 않음
- 높은 몬스터는 실제 Movement로 안전 지점을 선택

Prediction:

- 높은 몬스터는 이동 Target에 더 적절한 선행 판단을 보임
- hidden/unobserved 정보는 사용하지 않음

## 9. Phase 7 - AI Brain Rework

### 목표

현재 BT → temporary StateTree → StateName → DataTable RowName 연결을 제거하거나 명시적 구조로 대체한다.

### 반드시 해결할 것

- State 이름과 Pattern Row 이름의 암묵적 결합 제거
- AI Skill 실행이 Player Input simulation에 의존해야 하는지 검증
- Trait로 변한 최종 Skill Set을 AI가 사용
- Personality 값이 실제 판단에 연결
- Perception/Target/Decision/Movement/Skill execution 책임 분리

### Experiment A - StateTree Primary Brain

```
AIController
└ StateTreeAIComponent
  ├ Idle
  ├ Patrol
  ├ Alert
  ├ Combat
  ├ Reaction
  └ Dead
```

### Experiment B - BT + StateTree 분리

```
Behavior Tree
= 공통 AI flow

StateTree
= combat pattern/decision
```

샘플 Monster 1종에서 두 구조의 제작/디버깅 비용을 비교한다.

### 선택 기준

- 몬스터별 Pattern 제작 편의
- 기존 코드 재사용량
- AI 상태 추적/디버깅 편의
- Personality 연결 난이도
- Skill Set 변주 대응
- Boss/Elite 확장 가능성
- 중복 로직 발생 여부

## 10. Phase 8 - EQS

### 목표

공간 문제에만 선택적으로 도입한다.

첫 후보는 Evasion이다.

```
Threat
→ Evade decision
→ EQS candidate positions
→ score
→ move/dodge
```

그 다음 필요 시:

- ranged position
- line of sight
- retreat
- flank
- high-ground observation

으로 확장한다.

### Performance acceptance

EQS가 모든 AI Tick/Move에 상시 실행되지 않아야 한다.

Debug에서 Query 빈도를 확인할 수 있어야 한다.

## 11. Phase 9 - Trait Graph Editor

### 전제

Phase 1~5의 데이터 구조가 실제 콘텐츠에서 유지될 때 진행한다.

### 목표 UX

콘텐츠 브라우저:

```
TT_Weapon
TT_Body
TT_Element
```

`TT_Weapon`을 열면:

```
[Sword] → [Speed Sword] → [Extreme Speed Sword]
    └──→ [Strong Sword]
```

Node Details:

- Display name/id
- Complexity Cost
- Skill Delta
- Stat Delta
- Visual Delta

### Candidate UE Editor 구조

- `UEdGraph`
- `UEdGraphNode`
- `UEdGraphSchema`
- custom asset editor

### Required editor validation

- multiple parent connection rejection
- cycle rejection
- invalid connection rejection
- orphan/root visualization
- duplicate/stale Node identity detection
- compile/validation errors visible in editor

### Runtime rule

Runtime module은 Editor Graph class에 의존하지 않는다.

Editor 저장/compile 결과만 Runtime Asset 데이터로 제공한다.

## 12. Phase 10 - Group Behavior

### Deferred

개인성향과 Trait 시스템이 안정화되기 전 구현하지 않는다.

추후 별도 설계 후보:

- Rally
- Assistance
- Formation
- target sharing
- surrounding
- independent combat preference

현재 문서는 이것을 확장 가능성으로만 기록한다.

## 13. Debug/Testing 필수 항목

### Trait

- resolved path dump
- total Complexity dump
- Skill Delta result
- Stat Delta result
- Visual Delta result
- invalid tree validation

### Personality

AI Debug에서 개체의 7개 값을 볼 수 있어야 한다.

가능하면 현재 결정 이유도 표시한다.

예:

```
Action: Reposition
Reason: Caution threshold not met
ExpectedHit: 0.42
Caution: 0.85
```

### EQS

- query reason
- candidate point visualization
- selected point
- score

### StateTree/BT

현재 활성 상태와 선택 Skill/Action을 추적할 수 있어야 한다.

## 14. 실패/재검토 기준

다음 중 하나가 반복되면 Shared Trait Tree 설계를 다시 검토한다.

1. 대부분의 Trait가 다른 Trait의 종류를 알아야만 동작한다.
2. 실제 콘텐츠에서 Tree depth가 거의 사용되지 않는다.
3. Variant 결과를 이해하려면 수십 개 조합 예외 Table이 필요하다.
4. Skill System과 Trait Delta 연결을 위해 Monster 전용 우회 계층이 계속 늘어난다.
5. Visual Telegraph가 불가능해 동일 외형이 전혀 다른 능력을 반복적으로 가진다.

Personality도 다음 경우 재검토한다.

1. 두 값이 항상 같은 알고리즘을 조절한다.
2. 값 변화가 플레이에서 구분되지 않는다.
3. 값이 실제 판단이 아니라 랜덤 성공률로만 사용된다.

## 15. Codex 구현 규칙

1. 구현 시작 전 최신 Monster/Skill 관련 코드를 다시 확인한다.
2. 현재 코드에 동일 책임을 가진 타입이 있으면 재사용한다.
3. 기존 파일/클래스 이름을 불필요하게 바꾸지 않는다.
4. Decision을 임의로 변경하지 않는다.
5. 새 Manager/System을 추가하기 전에 기존 소유자를 확인한다.
6. Trait Editor를 Runtime Model보다 먼저 만들지 않는다.
7. StateTree 사용 자체를 목표로 삼지 않는다. AI 요구를 가장 잘 해결하는 구조를 선택한다.
8. Spawn Rework를 건드리지 않는다.
9. Mass/Smart Object/Group Behavior를 이번 첫 구현 범위에 넣지 않는다.
10. 구현 중 문서와 충돌하는 실제 코드 제약을 발견하면 우회 구현보다 먼저 차이를 기록한다.
