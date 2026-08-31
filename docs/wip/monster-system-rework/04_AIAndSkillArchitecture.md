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

> 현재 상황에서 누구를 대상으로 어떤 행동/Skill을 선택하고 언제 Commit할 것인가?

### 목표 데이터 흐름

```
Perception / Combat Events
        ↓
Team relation
        ↓
Target eligibility / combat entry
        ↓
AI Decision
        ↓
Skill Candidate / Action
        ↓
Personality / position / ally-risk evaluation
        ↓
Skill execution request
        ↓
MASkillManager
        ↓
Skill Module Composition
        ↓
GAS Ability
        ↓
central damage/effect resolution
```

## 2. Team은 AI보다 아래의 관계 사실

AI가 Goblin, Golem, Player 같은 구체 종족 이름을 보고 적을 정하지 않는다.

기본 관계는 다음 두 개만 필요하다.

```
Same Team
→ Ally

Different Team
→ Non-Team
```

Relationship Matrix는 초기 구조에 없다.

`Different Team`은 "공격 가능한 후보"일 뿐 "즉시 공격해야 하는 Enemy"라는 뜻이 아니다.

## 3. Combat Entry

### Decision: 선제 교전

```
Perceived Actor
→ Team check
→ Same Team?
   ├ Yes → Ally
   └ No  → Preemptiveness evaluation
             ├ pass → enter combat
             └ fail → observe / ignore
```

### Decision: 반격

Non-Team Actor가 직접적인 적대 행위를 한 경우 Preemptiveness와 별개로 Combat 진입이 가능해야 한다.

예:

```
Golem Team 3
Preemptiveness = 0.1

Goblin Team 2 attacks Golem
→ retaliation condition
→ Golem can enter combat against Goblin
```

Same Team Friendly Fire는 초기에는 Team 관계 자체를 Enemy로 바꾸지 않는다.

## 4. 현재 Behavior Tree와 StateTree

### Fact: 현재 BT가 Primary Brain

AIController는 Behavior Tree를 실행한다.

### Fact: 현재 StateTree는 Pattern Planner

BT Task가 StateTree를 임시 실행해 활성 State 이름을 수집하고 PatternData RowName으로 사용한다.

### Problem: State Name → DataTable RowName

이 연결은 타입 안전하지 않으며 에디터 Rename에도 취약하다.

### Problem: StateTree를 실제 상태 실행기로 사용하지 않는다

StateTree를 Start/Tick한 뒤 State 이름만 결과로 추출하므로 StateTree의 State/Transition/Task 실행 모델을 온전히 활용하지 않는다.

## 5. StateTree를 유지할 이유

### Candidate: StateTree는 Monster Combat Decision의 핵심 도구로 승격할 가치가 있다

MA는 몬스터가 비교적 복잡하게 싸우는 방향을 목표로 한다.

예:

- 공격/후퇴/재접근
- 여러 Skill 선택
- 위험 회피
- 거리 관리
- 공격 타이밍 판단
- 다른 Team Monster와의 전투
- Trait로 인해 달라진 Skill Set
- Personality로 달라지는 판단

### Open Question: BT를 제거할지 유지할지

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

샘플 구현에서 제작/디버깅 비용을 비교한다.

StateTree를 사용한다는 이유만으로 BT를 제거하지 않는다.

반대로 BT가 이미 존재한다는 이유만으로 현재의 StateName → RowName 임시 구조를 유지하지 않는다.

## 6. AI는 구체 Skill 이름에 얼마나 의존해야 하는가

### Candidate: AI는 가능한 한 Skill 내부 구현보다 사용 조건/의미를 본다

StateTree에 `UseFireball` 같은 구체 Skill 이름을 하드코딩하면 Trait가 Skill Set을 바꿀 때 AI 재사용성이 떨어진다.

가능한 정보:

- Skill Slot
- role/category
- range/use condition
- cooldown/availability
- targeting shape
- metadata/tag

정확한 메타데이터는 현재 Skill System을 먼저 확인한다.

새 Monster 전용 Skill Role 시스템을 선제 구현하지 않는다.

## 7. Trait 결과와 AI

### Decision: Trait는 Skill Set을 바꿀 수 있다

```
Goblin Base
+ Spear Trait
+ Fire Trait
→ final Skill Set
→ AI decision
```

AI가 `GoblinFireSpear` 같은 별도 Class를 알 필요가 없어야 한다.

### Candidate: Trait가 BT/StateTree 자체를 직접 교체하지 않는다

초기에는 Trait가 Skill/Stat/Visual을 변경하고 AI가 Resolved 결과를 사용한다.

실제 행동 구조 자체가 바뀌는 Trait 요구가 확인되면 Behavior Override를 검토한다.

## 8. Personality와 AI Decision

### Caution

```
candidate skill
→ range/timing/prediction evaluation
→ hit confidence
→ Caution threshold
→ execute or reposition
```

Caution은 Target 적중 가능성만 담당한다.

아군 피격 위험을 Caution에 섞지 않는다.

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
perceived Non-Team actor
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

## 9. Team Damage-aware Attack Evaluation

### Decision: 실제 Team Damage Rate를 AI 평가에 반영할 수 있어야 한다

같은 Team에게 피해가 들어가는 세션에서는 AI가 아군 위치를 무시하고 광역기/투사체를 난사하면 정교한 AI의 느낌이 깨진다.

기본 공격 가치 개념:

```
AttackValue
= EnemyHitValue
- AllyDamageCost
- Other tactical costs
```

여기서:

```
AllyDamageCost
∝ expected friendly damage
× TeamDamageRate
```

따라서:

```
TeamDamageRate = 0%
→ 실제 피해 비용은 0
→ Friendly Damage penalty도 최소

TeamDamageRate = 100%
→ 아군 예상 피해를 강하게 비용으로 평가
→ 사선 변경 / 대기 / 다른 Skill 선택 가능
```

이것은 "아군을 절대 맞히지 않는다"는 하드 룰이 아니다.

적 다수를 공격해 얻는 가치가 훨씬 크다면 아군 피해를 감수하는 판단도 가능해야 한다.

초기에는 몬스터별 별도 FriendlyFireDiscipline Personality를 만들지 않는다.

먼저 공통 비용 평가만으로 충분한지 확인한다.

## 10. EQS

### Candidate: EQS는 공간 판단 도구

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
- 아군을 피한 사선 확보 위치
- 거리 유지 위치
- 퇴각 지점
- 측면 위치
- 고지대 순찰/관측 지점 후보

모든 Move마다 EQS를 실행하지 않는다.

## 11. Evasion 예시

```
Projectile / Attack Threat
        ↓
Threat interpreted by AI
        ↓
Reaction
        ↓
Prediction
        ↓
AI chooses Evade action
        ↓
EQS / movement query
        ↓
Evasion controls evaluation quality
        ↓
Move / Dodge Skill
```

Evasion이 높을수록 회피 확률이 증가하는 것이 아니라 실제 Movement Solution의 품질이 높아진다.

## 12. Caution 예시

단순 AI:

```
Distance <= SkillRange
→ Attack
```

높은 Caution:

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

## 13. Target 정책

### Candidate: Target 선택은 Team + Combat Context를 먼저 본다

기본 후보 흐름:

```
Perceived Actors
→ remove Same-Team allies from hostile candidates
→ include Non-Team actors
→ current retaliation target / Preemptiveness
→ target utility
→ select or retain target
```

현재 유효 Target을 너무 쉽게 버리지 않는다.

Persistence가 Target 유지 판단에 영향을 줄 수 있다.

Threat/Hate Table은 실제 요구가 확인되기 전 만들지 않는다.

### Example: Monster vs Monster

```
Goblin Team 2 sees:
- Player Team 1
- Golem Team 3

둘 다 Non-Team
→ Preemptiveness/현재 Combat Context/Target Utility로 선택
```

AI가 Player를 특별한 유일 적으로 하드코딩하지 않아야 한다.

## 14. Damage Rule의 위치

### Decision: Team Damage Rate를 Skill마다 구현하지 않는다

Skill은 공격 결과를 발생시킨다.

Same-Team Damage Scale은 가능한 한 공통 Damage Resolution 지점에서 적용한다.

```
Skill hit
→ source Team
→ target Team
→ same/different
→ TeamDamageRate if same
→ final damage
```

정확한 GAS/Attribute 적용 지점은 구현 전 현재 Damage pipeline을 감사한 뒤 정한다.

## 15. Movement와 Level Rework 연결

Monster는 NavMesh를 사용한다.

Level Rework의 high-ground Monster는 upper NavMesh에서 순찰하다 Combat Target을 발견하면 lower Area로 Drop/Jump할 수 있다.

이 Traversal은 Monster별 임시 점프 코드보다 안정적인 NavLink/Traversal 데이터와 연결하는 방향을 우선한다.

## 16. 현재 리워크에서 피할 구조

- AI Class마다 공격 함수 추가
- StateTree State 이름과 DataTable Row 이름 암묵적 연결
- Trait마다 전용 Monster subclass 생성
- Player만 적으로 하드코딩
- TeamId마다 종족 관계 테이블 작성
- Different Team을 무조건 즉시 공격하도록 고정
- Team Damage Rate를 각 Skill에 개별 구현
- Player Input simulation을 AI Skill 실행의 필수 경로로 유지
- 모든 Spatial Decision에 EQS 실행
- Personality를 단순 성공 확률로 변환
- Skill System과 AI Decision을 하나의 거대한 Monster Component에 합치기
