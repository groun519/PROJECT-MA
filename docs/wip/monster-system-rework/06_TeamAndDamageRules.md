# 06 - Team and Damage Rules

## 1. 목적

이 문서는 Monster Rework에서 Team이 무엇을 의미하고, 다른 Team 간 전투와 같은 Team 간 Damage를 어떤 규칙으로 처리할지 정의한다.

목표는 복잡한 Faction/Diplomacy 시스템이 아니다.

목표는 다음 세 가지다.

1. Ally 판정을 단순하고 재사용 가능하게 만든다.
2. Goblin과 Golem처럼 서로 다른 Monster 집단이 실제 Skill System으로 자연스럽게 싸울 수 있게 한다.
3. Friendly Fire 강도를 맵/세션 규칙으로 조절하면서 Player와 Monster에 같은 기본 규칙을 적용한다.

## 2. Team의 의미

### Decision: Team은 Ally 판정의 최소 단위

```
MyTeam == OtherTeam
→ Ally

MyTeam != OtherTeam
→ Non-Team
```

Team은 호감도, 외교, 종족 관계를 표현하지 않는다.

초기에는 다음 데이터가 없다.

- Friendly / Neutral / Hostile 3단계 관계
- Goblin ↔ Golem 별도 호감도
- Faction Reputation
- Alliance Matrix
- Diplomacy Table

### 이유

Non-Team을 실제로 먼저 공격할지는 `Preemptiveness`가 이미 담당한다.

관계표를 추가하면 같은 의미를 두 군데에서 관리하게 된다.

## 3. Monster Set과 Team

### Decision: Monster Set은 Default Team을 가진다

예:

```
Player        Team 1
Goblin Set    Team 2
Golem Set     Team 3
Slime Set     Team 4
```

이는 "Goblin이라는 Class는 무조건 Team 2"라고 코드에 박는 것이 아니다.

```
Monster Set
├ Base stats
├ Base skills
├ Trait Trees
├ Base Personality
└ DefaultTeamId
```

처럼 데이터가 기본 Team을 제공하는 방향이다.

### Initial rule

Monster가 초기화될 때 Monster Set의 `DefaultTeamId`를 Runtime Team으로 사용한다.

Runtime Team Override는 실제 Encounter 요구가 생기기 전 별도 범용 시스템으로 만들지 않는다.

## 4. Player Team

### Decision

Player도 동일한 Team 판정 체계를 사용한다.

현재 협동 플레이의 기본적인 경우 같은 Player Team을 사용할 수 있다.

정확한 Player Team 배정 규칙은 GameMode/Session 구조를 감사한 뒤 결정한다.

Monster 전용 Team 시스템과 Player 전용 Team 시스템을 따로 만들지 않는다.

## 5. 다른 Team은 즉시 Enemy가 아니다

### Decision

```
Different Team
→ Non-Team
→ Preemptiveness
→ 선제 교전 여부
```

예:

```
Goblin
Team 2
Preemptiveness 0.9

Golem
Team 3
Preemptiveness 0.1
```

Goblin은 Golem을 먼저 공격하기 쉽다.

Golem은 Goblin을 발견해도 먼저 공격하지 않을 수 있다.

따라서 같은 Team 구조만으로도 몬스터 종족별 전투 성격을 만들 수 있다.

## 6. Retaliation

### Decision: 반격은 Preemptiveness와 분리

Preemptiveness가 낮은 몬스터가 아무 반격도 하지 않는 것은 의도가 아니다.

```
Non-Team attack received
→ attacker becomes valid retaliation target
→ enter combat
```

즉:

```
Preemptiveness
= 내가 먼저 싸움을 시작하는가

Retaliation
= 상대가 실제 적대 행위를 한 뒤 대응하는가
```

Retaliation의 세부 지속시간/Target 유지에는 Reaction/Persistence가 영향을 줄 수 있다.

### Same-Team hit

같은 Team의 Friendly Fire를 맞았다는 이유만으로 Ally를 Enemy로 전환하지 않는다.

향후 Betrayal 같은 별도 게임 규칙이 실제로 필요할 때만 확장한다.

## 7. Monster vs Monster 전투

### Decision

다른 Team의 Monster끼리 실제 Combat Target이 될 수 있어야 한다.

예:

```
Goblin Group - Team 2
Golem Group  - Team 3
Player       - Team 1
```

Player가 Goblin 집단을 Golem 근처로 유도하면:

```
Goblin detects Golem
→ Different Team
→ Goblin Preemptiveness evaluation

Golem detects Goblin
→ Different Team
→ Golem Preemptiveness evaluation

if combat begins
→ both use normal MA Skill System
→ normal Damage pipeline
```

Monster-vs-Monster용 가짜 Damage/연출 시스템을 만들지 않는다.

## 8. Team Damage Rate

### Decision: 맵/세션에서 0~100%로 설정 가능

사용자-facing 설정:

```
Team Damage Rate = 0 ~ 100 %
```

Runtime 내부 표현 후보:

```
0.0 ~ 1.0
```

정확한 저장 타입은 현재 Map/Match Setting 구조를 본 뒤 결정한다.

### Damage rule

```
if SourceTeam == TargetTeam:
    FinalDamage = BaseDamage * TeamDamageRate
else:
    FinalDamage = BaseDamage
```

예:

```
TeamDamageRate = 0%

Player → allied Player = 0%
Goblin → allied Goblin = 0%
Goblin → Golem         = 100%
```

```
TeamDamageRate = 25%

Player → allied Player = 25%
Goblin → allied Goblin = 25%
Goblin → Golem         = 100%
```

```
TeamDamageRate = 100%

Player → allied Player = 100%
Goblin → allied Goblin = 100%
Goblin → Golem         = 100%
```

## 9. Player와 Monster에 같은 Rule 적용

### Decision

Team Damage Rule은 Actor 종류에 따라 갈라지지 않는 것을 목표로 한다.

피해야 할 구조:

```
PlayerFriendlyFireRate
MonsterFriendlyFireRate
GoblinFriendlyFireRate
BossFriendlyFireRate
...
```

기본 규칙은 하나다.

```
Same Team
→ Team Damage Rate
```

특정 Boss가 아군 피해를 무시하고 잡몹을 죽이는 특별 규칙이 실제로 필요하면 그때 명시적 예외를 추가한다.

## 10. Authority와 Replication

### Decision: 서버가 규칙을 소유

멀티플레이 Damage 결과는 Server Authority가 결정해야 한다.

목표:

```
Host/Match setting
→ server-owned TeamDamageRate
→ clients receive replicated/read-only value as needed
→ server resolves final damage
```

Client가 자신의 Team Damage Rate를 따로 적용하지 않는다.

정확한 소유 객체는 현재 Session/GameState/Map Setting 구조를 확인한 뒤 결정한다.

## 11. 중앙 Damage 적용

### Decision

Team Damage Rate는 개별 Skill에 직접 넣지 않는다.

잘못된 예:

```
Fireball:
if same team → damage * rate

Sword:
if same team → damage * rate

Beam:
if same team → damage * rate
```

목표:

```
Any Skill / Damage Source
→ common Damage Resolution
→ Source Team / Target Team
→ Team Damage Rate
→ Final Damage
```

이렇게 해야 새로운 Skill Module이 추가되어도 Team Damage 지원 코드를 반복하지 않는다.

### Implementation audit

현재 GAS Damage 적용 흐름을 먼저 확인한다.

가능하면 Damage가 Attribute에 최종 반영되기 직전의 공통 지점을 사용한다.

정확한 클래스/함수는 코드 감사 전 문서에서 고정하지 않는다.

## 12. Crowd Control / Status Effect

### Open Question

`Team Damage Rate`는 이름 그대로 Damage 비율을 정의한다.

다음은 자동으로 결정하지 않는다.

- Stun
- Knockback
- Freeze
- Root
- Burn/DoT
- Debuff

예를 들어 Team Damage 10%인데 Stun은 100% 들어가면 체감상 Friendly Fire가 매우 강할 수 있다.

반대로 모든 CC를 TeamDamageRate에 기계적으로 곱하는 것도 올바른 설계라고 확정할 수 없다.

따라서 첫 Team Damage 구현에서는:

1. 현재 Skill/Effect pipeline에서 Damage와 CC가 어떻게 분리되어 있는지 확인한다.
2. Damage Rate 구현을 먼저 독립적으로 완료한다.
3. 실제 플레이 테스트 후 Team CC Rule을 별도 결정한다.

Team Damage Rate가 CC 정책까지 암묵적으로 소유하지 않게 한다.

## 13. AI Friendly Fire Awareness

### Decision: AI가 같은 Team의 예상 피해를 평가할 수 있어야 한다

Team Damage Rate가 0보다 클 때 AI가 아군을 완전히 무시하면 다음 문제가 생긴다.

- 원거리 Monster가 앞의 Ally를 계속 관통/타격
- 대형 AoE를 Ally 중심에 반복 사용
- 100% Friendly Fire 세션에서도 AI가 자해성 전투를 반복

기본 Attack Evaluation:

```
EnemyHitValue
- FriendlyDamageCost
= AttackValue
```

```
FriendlyDamageCost
≈ expected ally damage
× TeamDamageRate
```

### TeamDamageRate에 따른 변화

```
0%
→ 피해 관점에서는 Ally overlap penalty 거의 없음

50%
→ 아군 피해와 적중 이득을 비교

100%
→ 아군 예상 피해의 실제 비용이 커짐
→ 다른 사선/위치/Skill을 찾을 가치 증가
```

### Important

AI에게 "아군이 있으면 공격 금지"를 하드코딩하지 않는다.

예:

```
Enemy 5명을 강한 AoE로 처치 가능
Ally 1명에게 작은 Friendly Damage 예상

→ 총 전투 가치가 높다면 공격 가능
```

이렇게 해야 팀킬률이 높은 모드에서도 AI가 과도하게 수동적이지 않다.

## 14. Friendly Fire Awareness와 Personality

### Decision: Caution에 합치지 않는다

Caution:

> 공격이 Target에게 실제로 닿을 가능성을 얼마나 엄격하게 보는가.

Friendly Fire Awareness:

> 공격으로 인해 Ally에게 발생할 비용을 어떻게 평가하는가.

둘은 다른 질문이다.

### Deferred: Friendly Fire Discipline

초기에는 모든 AI가 실제 `TeamDamageRate` 기반 비용을 공통적으로 평가한다.

추후 다음 차이가 게임적으로 필요해질 경우에만 별도 Personality를 검토한다.

- 아군 피해를 극도로 꺼리는 정예 병사
- 잡몹 피해를 신경 쓰지 않는 난폭한 Monster
- 자기 Ally까지 희생시키는 Boss

현재 Individual Behavior 7종에는 넣지 않는다.

## 15. Friendly Fire를 이용한 Player 전략

이 시스템은 단순 제약이 아니라 플레이 수단이 될 수 있다.

가능한 상황:

- 다른 Team Monster 집단을 서로 만나게 하기
- Golem의 강한 공격을 Goblin 쪽으로 유도
- 몬스터 사이에 위치해 공격 각을 꼬기
- Team Damage 100% 세션에서 Enemy AoE로 다른 Enemy를 맞히기
- Team Damage 0% 세션에서는 순수 핵앤슬래시 플레이에 집중

Team Damage Rate가 방/맵 설정인 이유는 이런 플레이 성향을 사용자가 선택할 수 있게 하기 위해서다.

## 16. Group Behavior와의 경계

Team은 Group AI가 아니다.

현재 Team이 제공하는 정보:

```
Who is Ally?
Who is Non-Team?
```

나중 Group Behavior가 제공할 수 있는 것:

```
Should allies share target?
Should I join ally combat?
Should I support ally?
Should we form a formation?
Should we surround the target?
```

따라서 Team 기반은 지금 만들 수 있지만 Rally/Formation/Support 같은 Group Behavior는 계속 Deferred다.

## 17. Debug 요구

최소한 다음 정보를 AI/Combat Debug에서 확인할 수 있어야 한다.

```
Actor: Goblin_12
Team: 2

Target: Golem_03
Target Team: 3
Relation: Non-Team

Preemptiveness: 0.82
Combat Entry: Preemptive

TeamDamageRate: 0.50
```

Damage Debug 예:

```
Source: Goblin_12 Team 2
Target: Goblin_09 Team 2
BaseDamage: 100
TeamDamageRate: 0.25
FinalDamage: 25
```

Attack Evaluation 예:

```
Skill: Explosion
EnemyValue: 180
FriendlyDamageCost: 40
FinalAttackValue: 140
Decision: Execute
```

## 18. Acceptance Criteria

1. 같은 Team Actor는 Ally로 일관되게 판정된다.
2. 다른 Team Actor는 별도 종족 관계표 없이 Non-Team으로 판정된다.
3. Preemptiveness 0에 가까운 Monster는 Non-Team을 보고도 먼저 공격하지 않을 수 있다.
4. Preemptiveness가 낮아도 Non-Team에게 공격받으면 정상 반격할 수 있다.
5. Goblin과 Golem이 서로 다른 Team이면 실제 Skill System으로 서로 싸울 수 있다.
6. TeamDamageRate 0/0.25/1.0에서 동일 Damage 입력의 최종 결과가 정확하다.
7. Player와 Monster가 같은 Team Damage 계산 경로를 사용한다.
8. 새로운 Skill을 추가해도 Skill 코드에 Team Damage 특수 처리를 추가할 필요가 없다.
9. Team Damage가 높은 세션에서 AI가 Ally 피해 위험을 공격 가치에 반영할 수 있다.
10. Same-Team Friendly Fire가 Team 관계 자체를 적대로 바꾸지 않는다.

## 19. 현재 보류

- Diplomacy / Relationship Matrix
- Faction Reputation
- Runtime alliance changes
- Betrayal
- Team별 별도 Friendly Fire Rate
- Friendly Fire Discipline Personality
- Group rally/support/formation
- Team Crowd Control Rate

실제 콘텐츠 요구가 생기기 전 구현하지 않는다.
