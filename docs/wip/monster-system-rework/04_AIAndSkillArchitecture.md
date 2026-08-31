# 04 - AI and Skill Architecture

## 1. 핵심 책임 경계

### Decision: Skill System은 실제 전투 행동 실행 계층

몬스터가 기존 MA Skill System을 사용하는 것은 이번 리워크의 핵심 전제다.

AI가 직접 다음을 구현하지 않는다.

- Fireball damage
- Slash hit logic
- AoE effect application
- Buff/Debuff effect
- Skill-specific projectile behavior

이 기능들은 기존 Skill Module / Skill Manager / GAS 계층이 소유해야 한다.

AI의 질문은 다음에 가깝다.

> 현재 상황에서 어떤 행동/Skill을 선택하고 언제 Commit할 것인가?

### 목표 데이터 흐름

```
Perception / Combat Context
        ↓
AI Decision
        ↓
Skill Candidate / Action
        ↓
Caution / Personality / Position evaluation
        ↓
Skill execution request
        ↓
MASkillManager
        ↓
Skill Module Composition
        ↓
GAS Ability
        ↓
actual gameplay effect
```

## 2. 현재 Behavior Tree와 StateTree

### Fact: 현재 BT가 Primary Brain

AIController는 Behavior Tree를 실행한다.

### Fact: 현재 StateTree는 Pattern Planner

BT Task가 StateTree를 임시 실행해 활성 State 이름을 수집하고 PatternData RowName으로 사용한다.

### Problem: State Name → DataTable RowName

이 연결은 타입 안전하지 않으며 에디터 Rename에도 취약하다.

### Problem: StateTree를 실제 상태 실행기로 사용하지 않는다

StateTree를 Start/Tick한 뒤 State 이름만 결과로 추출하므로 StateTree의 State/Transition/Task 실행 모델을 온전히 활용하지 않는다.

## 3. StateTree를 유지할 이유

### Candidate: StateTree는 Monster Combat Decision의 핵심 도구로 승격할 가치가 있다

MA는 몬스터가 비교적 복잡하게 싸우는 방향을 목표로 한다.

예:

- 공격/후퇴/재접근
- 여러 Skill 선택
- 위험 회피
- 거리 관리
- 공격 타이밍 판단
- Trait로 인해 달라진 Skill Set
- Personality로 달라지는 판단

이런 명시적인 상태 전환은 StateTree와 궁합이 좋다.

또한 프로젝트에 StateTree/GameplayStateTree 플러그인이 이미 존재한다.

### Open Question: BT를 제거할지 유지할지

아직 최종 결정하지 않는다.

후보 A:

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

후보 B:

```
Behavior Tree
= 공통 이동/전투 오케스트레이션

StateTree
= Monster Combat Pattern/Decision
```

현재 코드가 BT 기반으로 이미 동작한다는 비용과 향후 Monster 복잡도를 둘 다 고려해 샘플 리워크에서 비교한다.

중요:

StateTree를 사용한다는 이유만으로 BT를 제거하지 않는다.

반대로 BT를 이미 사용한다는 이유만으로 현재 임시 StateTree Pattern 구조를 유지하지도 않는다.

## 4. AI는 구체 Skill 이름에 얼마나 의존해야 하는가

### Candidate: AI는 가능한 한 Skill 내부 구현보다 사용 조건/의미를 본다

예를 들어 StateTree에 `UseFireball` 같은 구체 Skill 이름을 하드코딩하면 Trait가 Skill Set을 바꿀 때 AI 재사용성이 떨어진다.

가능한 방향:

- Skill Slot
- Skill role/category
- range/use condition
- cooldown/availability
- targeting shape
- metadata/tag

정확히 어떤 메타데이터를 쓸지는 현재 Skill System을 기준으로 결정해야 한다.

새로운 Monster 전용 Skill Role 시스템을 먼저 만들지 않는다.

### Implementation rule

Codex는 먼저 현재 SkillManager/SkillModule이 이미 노출하는 정보로 AI 선택이 가능한지 확인한다.

부족한 정보가 명확할 때만 최소 메타데이터를 추가한다.

## 5. Trait 결과와 AI

### Decision: Trait는 Skill Set을 바꿀 수 있다

AI는 최종 Resolved Skill Set을 기준으로 행동한다.

예:

```
Goblin Base
+ Spear Trait
+ Fire Trait
→ final Skill Set
→ AI decision
```

AI가 `GoblinFireSpear` 같은 별도 Class를 알아야 하는 구조를 피한다.

### Candidate: Trait가 Behavior Tree/StateTree를 직접 교체하지 않는다

초기에는 Trait가 Skill/Stat/Visual만 바꾸고 AI는 그 결과를 해석한다.

정말 행동 구조 자체가 바뀌어야 하는 Trait가 실제로 생길 때 Behavior Override를 검토한다.

## 6. Personality와 AI Decision

Personality는 별도 Skill을 제공하지 않는다.

같은 Skill Set에 대한 판단 방식만 변화시킨다.

예:

### Caution

```
candidate skill
→ range/timing/prediction evaluation
→ hit confidence
→ Caution threshold
→ execute or reposition
```

### Initiative

```
combat state
→ opportunity search pressure
→ active reposition/approach frequency
```

### Persistence

```
current goal
→ switch/abandon threshold
```

### Preemptiveness

```
perceived non-team actor
→ engagement threshold
```

### Reaction

```
event perceived
→ response delay/priority
```

### Prediction

```
observed motion/action
→ estimated future state
```

### Evasion

```
threat response selected
→ quality of dodge route/point selection
```

## 7. EQS

### Candidate: EQS는 공간 판단 도구

EQS는 Brain이 아니다.

역할:

```
AI / StateTree
= 무엇을 할 것인가

EQS
= 어디에서 할 것인가

Navigation / Movement
= 어떻게 이동할 것인가

Skill System
= 실제 Action을 어떻게 실행할 것인가
```

적합한 사용 예:

- 투사체/공격 Area 회피 위치
- 원거리 공격 위치
- LOS 확보 위치
- 거리 유지 위치
- 퇴각 지점
- 측면 위치
- 고지대 순찰/관측 지점 후보

### Performance rule

모든 Move마다 EQS를 실행하지 않는다.

공간 점수화가 실제로 필요한 Tactical Decision에만 사용한다.

## 8. Evasion 예시

목표:

Evasion이 높을수록 "회피 확률"이 아니라 더 좋은 실제 Movement Route를 선택한다.

```
Projectile / Attack Threat
        ↓
Threat interpreted by AI
        ↓
Reaction decides response timing
        ↓
Prediction estimates future danger if possible
        ↓
AI chooses Evade action
        ↓
EQS / movement query scores candidates
        ↓
Evasion controls evaluation quality/strictness
        ↓
Move / Dodge Skill
```

논타겟 공격에서 높은 Evasion 몬스터는 플레이어의 조준/예측 능력을 요구하게 만든다.

대신 해당 공격을 맞혔을 때 높은 보상을 설계할 수 있다.

정확한 보상 수치는 Monster Rework 범위에서 결정하지 않는다.

## 9. Caution 예시

단순 사거리 체크:

```
Distance <= SkillRange
→ Attack
```

높은 Caution에서는:

```
current distance
+ windup
+ hit timing
+ target velocity
+ Prediction
+ attack shape/range
→ expected hit quality
→ commit
```

이를 통해 AI가 공격 사거리 끝에서 무의미한 헛공격을 반복하는 문제를 줄일 수 있다.

## 10. Target 정책

### Candidate: 현재 유효 Target을 쉽게 버리지 않는다

현재 AI는 일정 주기로 더 가까운 적을 찾을 수 있다.

멀티플레이에서 너무 자주 Target을 바꾸면 행동이 불안정해질 수 있다.

초기 후보 정책:

- 현재 Target이 살아 있고 유효하면 유지
- 시야 상실/사망/교전 포기 조건에서 재선택
- Persistence가 Target 유지 판단에 영향을 줄 수 있음

Threat/Hate Table 같은 복잡한 시스템은 실제 요구가 확인되기 전 만들지 않는다.

## 11. Movement와 Level Rework 연결

Monster는 NavMesh를 사용한다.

Level Rework의 high-ground Monster는 upper NavMesh에서 순찰하다 Player를 발견하면 lower Area로 Drop/Jump할 수 있다.

이 Traversal은 Monster별 임시 점프 코드보다 안정적인 NavLink/Traversal 데이터와 연결하는 방향을 우선한다.

Automatic Navigation Link Generation 같은 실험 기능을 필수 기반으로 두지 않는다.

## 12. 현재 리워크에서 피할 구조

- AI Class마다 공격 함수 추가
- StateTree State 이름과 DataTable Row 이름 암묵적 연결
- Trait마다 전용 Monster subclass 생성
- Player Input simulation을 AI Skill 실행의 필수 경로로 유지
- 모든 Spatial Decision에 EQS 실행
- Personality를 단순 성공 확률로 변환
- Skill System과 AI Decision을 하나의 거대한 Monster Component에 합치기
