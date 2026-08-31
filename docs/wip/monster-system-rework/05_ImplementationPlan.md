# 05 - Implementation Plan

## 1. 구현 전략

Monster Rework를 한 번에 완성하지 않는다.

의존성이 낮고 다른 기능이 기대는 기반부터 검증한다.

구현 순서의 핵심은 다음과 같다.

```
0. Current code audit
1. Team foundation
2. Team Damage rule
3. Trait runtime model
4. Monster Set + Resolver
5. Skill Delta integration
6. Stat / Visual integration
7. Complexity selection
8. Individual Personality data
9. Target / Combat Entry
10. AI Brain rework
11. Caution / Prediction / Reaction
12. Evasion + EQS
13. Team Damage-aware Attack Evaluation
14. Trait Graph Editor
15. Group Behavior later
```

이 순서는 "중요도 순서"가 아니라 **구현 의존성 순서**다.

Spawn은 현재 별도로 해결되었으므로 범위에 포함하지 않는다.

## 2. Phase 0 - 최신 코드 감사

### 목표

목업을 코드에 억지로 맞추지 않고 현재 프로젝트가 이미 가진 책임을 확인한다.

### 반드시 확인

- `AMonster`
- `AMACharacter`
- `AMAAIController`
- AI Perception affiliation/target filtering
- 현재 Team 또는 Generic Team 관련 코드 존재 여부
- Monster Skill Slot 데이터
- `UMASkillManagerComponent`
- Skill Module / Assembler
- 실제 Damage가 Attribute에 반영되는 공통 경로
- GameplayEffect / Damage Effect 적용 지점
- Pattern StateTree 관련 코드
- Map/Match/Session Setting 소유 구조
- GameMode / GameState 중 서버 룰 저장 위치
- 현재 Spawn Rework 결과와 Monster 초기화 시점
- 기존 Monster Blueprint/Data authoring 경로

### Output

구현 전 짧은 Audit Note를 남긴다.

```
Existing owner
Reusable type
Missing responsibility
Legacy responsibility to remove
Exact 5.8 API constraints
```

### Rule

기존에 같은 책임의 타입이 있으면 새 Manager를 만들지 않는다.

## 3. Phase 1 - Team Foundation

### 목표

Combatant가 최소한의 Team 관계를 일관되게 판정할 수 있게 한다.

### Step 1 - Runtime Team 표현 결정

현재 코드에 Team 구현이 있으면 재사용한다.

없다면 UE의 기존 Team 지원 구조를 사용할지 프로젝트 전용 최소 타입을 둘지 Audit 결과로 결정한다.

### Step 2 - 관계 함수 하나로 통일

필요한 의미는 초기에는 두 개다.

```
Same Team
Different Team
```

여러 AI/Skill에서 각자 Team 비교 코드를 만들지 않는다.

### Step 3 - Monster Set DefaultTeamId 설계 반영

Monster Set이 생기기 전이라면 데이터 계약만 정의하고,
실제 Set 구현 단계에서 연결한다.

### Step 4 - Player도 동일 Team 경로 사용

Monster 전용 Team 판정과 Player 전용 Team 판정을 따로 만들지 않는다.

### Acceptance

- Same Team 판정 테스트
- Different Team 판정 테스트
- Player/Monster 조합 테스트
- Team 미설정 Actor fallback 명시

## 4. Phase 2 - Team Damage Rule

### 목표

Same-Team Damage 배율을 한 중앙 경로에서 적용한다.

### Step 1 - Setting owner 결정

후보는 Map/Match/Session 설정 계층이다.

조건:

- Host가 0~100% 설정 가능
- Server가 권위 값 소유
- Client가 필요한 경우 읽을 수 있음
- Skill Asset마다 값 복제 금지

### Step 2 - 내부 값 정의

UI:

```
0 ~ 100 %
```

Runtime 후보:

```
0.0 ~ 1.0
```

### Step 3 - 중앙 Damage pipeline에 연결

```
Damage event
→ Source Team
→ Target Team
→ same?
→ TeamDamageRate
→ final damage
```

Skill마다 개별 처리하지 않는다.

### Step 4 - 0 / 25 / 100% 테스트

Base Damage 100 기준:

```
Rate 0.00 → Same Team 0
Rate 0.25 → Same Team 25
Rate 1.00 → Same Team 100
Different Team → always 100
```

### Step 5 - CC/Status는 건드리지 않음

Damage 외 Effect 정책은 먼저 감사하고 별도 결정한다.

### Acceptance

새 Skill Module이 Team Damage를 전혀 몰라도 동일 규칙이 적용되어야 한다.

## 5. Phase 3 - Trait Runtime Data Model

### 목표

UI 없이 Shared Trait Tree의 의미를 표현한다.

### 최소 데이터

- Trait Tree Asset
- Trait Node
- Parent
- Complexity Cost
- Skill Delta
- Stat Delta
- Visual Delta

### Validation

- Parent 최대 1
- cycle 금지
- invalid parent 금지
- stable Node identity
- Root validation

### Important

Node별 개별 DataAsset을 만들지 않는다.

```
TT_Weapon
└ Nodes[]
```

### Acceptance

```
Sword
├ Speed Sword
│  └ Extreme Speed Sword
└ Strong Sword
```

에서 `Sword → Speed Sword → Extreme Speed Sword`를 resolve하면 세 Delta가 순서대로 누적된다.

## 6. Phase 4 - Monster Set + Resolver

### 목표

Monster 종류의 기본 정체성을 하나의 데이터 계약으로 모은다.

### 최소 계약 후보

```
Monster Set
├ Base stats
├ Base skills
├ Trait Tree references
├ Base Personality
└ DefaultTeamId
```

실제 필드 배치는 기존 프로젝트 데이터 구조를 먼저 재사용한다.

### Resolver 흐름

```
Monster Set
→ Default Team
→ Trait Tree references
→ selected paths
→ merge deltas
→ Resolved Monster
```

### 샘플

최소:

- Goblin Team 2
- Slime Team 4
- Golem Team 3

공용 Tree:

- Weapon
- Body
- Element

### Acceptance

1. Goblin/Golem이 같은 Weapon Tree를 공유한다.
2. Slime은 Weapon Tree 없이 정상이다.
3. 각각 Default Team이 Runtime Actor에 반영된다.
4. Team이 Trait 종류에 종속되지 않는다.

## 7. Phase 5 - Skill Delta Integration

### 목표

Trait가 실제 MA Skill System을 변경한다.

### Rule

Monster 전용 Skill System을 만들지 않는다.

기존 Skill Slot / Skill Module / Assembler / GAS를 사용한다.

### Implementation

현재 샘플 Trait에 필요한 연산만 구현한다.

후보:

- Add
- Remove
- Replace
- parameter/module override

전부 선제 구현하지 않는다.

### Acceptance

같은 Goblin Base가 Trait 경로에 따라 실제 SkillManager에서 다른 Skill 구성을 가진다.

## 8. Phase 6 - Stat / Visual Integration

### Stat

기존 Attribute/GameplayEffect 경로를 우선 사용한다.

### Visual

최소 하나 이상의 주요 Trait가 외형으로 읽혀야 한다.

예:

- Sword → Weapon Mesh
- Fire → Material/VFX
- Large → Scale/Silhouette

### Acceptance

Debug UI 없이도 주요 Trait 차이를 시각적으로 구분할 수 있다.

## 9. Phase 7 - Complexity Selection

### 목표

Skill 직접 Random 지급 없이 Complexity Budget으로 Trait를 선택한다.

### Requirements

- valid path only
- budget 초과 금지
- parent 누적 Cost
- Seed 사용 시 reproducible
- 선택 결과 Debug dump

### Experiment

수백 회 생성하여 다음 분포를 확인한다.

- 깊은 단일 Tree
- 여러 얕은 Tree
- 특정 Tree 편향
- 사용되지 않는 Node

Encounter Cost와 직접 환산 공식은 아직 만들지 않는다.

## 10. Phase 8 - Individual Personality Data

### 목표

7개 값을 독립 데이터로 표현한다.

```
Prediction
Reaction
Evasion
Caution
Initiative
Persistence
Preemptiveness
```

### First requirement

아직 AI 알고리즘을 모두 연결하지 않아도 Runtime 개체별 값 조회와 Debug 표시가 가능해야 한다.

### Optional experiment

```
Base Personality
+ Trait Modifier
= Resolved Personality
```

은 실제 필요 Trait가 확인될 때만 추가한다.

## 11. Phase 9 - Target / Combat Entry

### 목표

Player-only Target 개념에서 벗어나 Team 기반 Candidate를 만든다.

### Step 1 - Perception 결과를 Team 기준으로 분류

```
Perceived Actor
→ Same Team = Ally
→ Different Team = Non-Team candidate
```

### Step 2 - Preemptiveness 연결

```
Non-Team detected
→ Preemptiveness
→ preemptive combat entry or no combat
```

### Step 3 - Retaliation 연결

```
Non-Team damages me
→ retaliation target
→ combat entry
```

Preemptiveness가 낮아도 반격할 수 있어야 한다.

### Step 4 - Current Target 유지

Persistence를 Target 유지/포기 조건에 연결할 수 있다.

Threat Table은 아직 만들지 않는다.

### Acceptance Scenario

```
Goblin Team 2, Preemptiveness high
Golem Team 3, Preemptiveness low
Player Team 1
```

- Goblin이 Golem을 먼저 공격 가능
- Golem은 발견만 했을 때는 무시 가능
- 공격받으면 Goblin에 반격
- Player도 둘 모두의 Non-Team 후보가 될 수 있음

## 12. Phase 10 - AI Brain Rework

### 목표

현재:

```
BT
→ temporary StateTree
→ StateName
→ DataTable RowName
```

암묵적 연결을 제거한다.

### Experiment A

StateTree primary brain.

### Experiment B

BT macro flow + StateTree combat decision.

### 비교 기준

- 기존 코드 재사용량
- 상태 추적/디버깅
- Trait Skill Set 대응
- Personality 연결
- Monster-vs-Monster Target 대응
- Boss/Elite 확장
- 중복 로직

### Must solve

AI가 Skill 내부 구현을 직접 소유하지 않는다.

AI Skill 실행이 Player Input simulation에 의존해야 하는지도 이 단계에서 제거/검증한다.

## 13. Phase 11 - Caution / Prediction / Reaction

세 값은 서로 의존 관계가 있으므로 같은 샘플 전투에서 검증한다.

### Prediction

현재 관측 정보로 Target의 미래 위치/상태를 추정.

### Caution

예측 결과와 Skill 사거리/시간을 이용해 실제 적중 가능성을 평가.

### Reaction

위험/기회 인지 후 행동 시작 타이밍을 조절.

### Acceptance

- Prediction이 높아도 hidden info는 사용하지 않음
- Caution 높은 AI가 사거리 끝 헛공격을 줄임
- Reaction 차이가 행동 시작 지연으로 체감됨

## 14. Phase 12 - Evasion + EQS

### 목표

회피를 확률이 아니라 실제 Movement 판단으로 만든다.

```
Threat
→ Evade decision
→ EQS candidate positions
→ score
→ Evasion quality
→ movement/dodge
```

### Test

- projectile
- melee telegraph
- AoE
- 좁은 공간
- 도망갈 곳 없음

EQS는 모든 Tick에 실행하지 않는다.

## 15. Phase 13 - Team Damage-aware Attack Evaluation

### 전제

Phase 2 Team Damage와 Phase 9 Team/Target이 정상 작동해야 한다.

### 목표

AI가 자신의 공격으로 Ally에게 발생할 실제 비용을 평가한다.

### Step 1 - Skill 영향 영역/사선에서 Ally 예상 피격 계산

정확한 계산 수준은 Skill metadata가 제공하는 범위 내에서 시작한다.

### Step 2 - TeamDamageRate 반영

```
FriendlyDamageCost
= expected ally damage × TeamDamageRate
```

### Step 3 - Enemy Hit Value와 비교

```
AttackValue
= EnemyHitValue - FriendlyDamageCost
```

### Step 4 - 재배치 선택 연결

필요하면 EQS로:

- clear line of fire
- side position
- safer AoE angle

을 찾는다.

### Acceptance

TeamDamageRate가 높아질수록 동일 상황에서 AI가 Ally를 맞히는 공격을 더 비싸게 평가해야 한다.

### Deferred

FriendlyFireDiscipline Personality는 아직 추가하지 않는다.

## 16. Phase 14 - Trait Graph Editor

### 전제

Runtime Model/Resolver/실제 Trait 콘텐츠가 안정화된 뒤 진행한다.

### UX

```
TT_Weapon

[Sword] → [Speed Sword] → [Extreme Speed Sword]
    └──→ [Strong Sword]
```

Node Details:

- ID / Display Name
- Complexity Cost
- Skill Delta
- Stat Delta
- Visual Delta

### Validation

- multiple parent reject
- cycle reject
- invalid connection
- orphan/root visualization
- duplicate/stale ID
- compile/validation feedback

Runtime module은 Editor Graph class에 의존하지 않는다.

## 17. Phase 15 - Group Behavior

### Deferred

Team은 이미 구현되어 있어도 Group AI는 별도다.

후보:

- Rally
- Assistance
- Support preference
- Formation
- Target sharing
- Surround
- Independent combat preference

Individual Behavior와 Team 기반이 충분히 안정화된 뒤 별도 목업으로 확장한다.

## 18. Debug / Testing

### Team

```
MyTeam
TargetTeam
Relation
CombatEntryReason
```

### Team Damage

```
BaseDamage
SameTeam?
TeamDamageRate
FinalDamage
```

### Trait

- resolved path
- Complexity
- Skill Delta
- Stat Delta
- Visual Delta

### Personality

7개 값과 현재 행동 이유.

### AI

```
Target
Target Team
Current State
Selected Skill
ExpectedHit
FriendlyDamageCost
FinalAttackValue
```

### EQS

- query reason
- candidates
- scores
- selected point

## 19. End-to-End 검증 시나리오

### Scenario A - Trait 재사용

Goblin과 Golem이 동일 Weapon Tree의 다른 경로를 사용.

### Scenario B - 성향 차이

동일 Skill/Traits를 가진 두 Monster가 Caution/Evasion 차이로 다른 전투를 보임.

### Scenario C - 종족 간 교전

```
Goblin Team 2
Golem Team 3
```

Goblin이 선공하고 Golem이 반격.

### Scenario D - Team Damage 0%

같은 Team 공격 피해 없음.

AI는 Damage 관점에서 Ally overlap을 거의 비용으로 보지 않음.

### Scenario E - Team Damage 100%

같은 Team도 정상 Damage.

AI는 아군 피격이 큰 공격을 더 비싸게 평가하고 다른 사선/Skill을 고려.

### Scenario F - Player 유도

Player가 Goblin과 Golem을 같은 공간으로 유도하여 Monster-vs-Monster 전투가 발생.

별도 scripted event 없이 Team + Personality + Perception만으로 발생해야 한다.

## 20. 실패/재검토 기준

### Trait

1. 대부분 Trait가 다른 Trait를 알아야 동작한다.
2. Tree depth가 실제 콘텐츠에서 거의 사용되지 않는다.
3. 조합 예외표가 계속 증가한다.
4. Skill System 우회 계층이 늘어난다.

### Personality

1. 두 값이 항상 같은 알고리즘을 조절한다.
2. 값 변화가 플레이에서 구분되지 않는다.
3. 실제 판단 대신 랜덤 확률로만 사용된다.

### Team

1. 종족마다 별도 관계표가 필요해진다.
2. Player/Monster Damage 경로가 갈라진다.
3. Skill마다 Friendly Fire 처리를 반복한다.
4. Same-Team hit만으로 AI가 무의미한 내전을 반복한다.
5. TeamDamageRate가 실제 Damage 외 너무 많은 규칙(CC, 관계, 어그로)을 동시에 소유하게 된다.

## 21. Codex 구현 규칙

1. 구현 시작 전 최신 Monster/Skill/Team/Damage 코드를 감사한다.
2. 기존 동일 책임 타입을 재사용한다.
3. 기존 파일/클래스 이름을 불필요하게 바꾸지 않는다.
4. Decision을 임의로 변경하지 않는다.
5. 새 Manager/System 전에 현재 소유자를 확인한다.
6. Team Damage는 중앙 Damage 경로에 둔다.
7. Different Team = 즉시 Enemy로 하드코딩하지 않는다.
8. Preemptiveness와 Retaliation을 같은 값으로 처리하지 않는다.
9. Trait Editor를 Runtime Model보다 먼저 만들지 않는다.
10. StateTree 사용 자체를 목표로 삼지 않는다.
11. Spawn Rework를 건드리지 않는다.
12. Mass/Smart Object/Relationship Matrix/Group Behavior를 첫 구현 범위에 넣지 않는다.
13. 실제 코드 제약이 문서와 충돌하면 우회 구현보다 차이를 먼저 기록한다.
