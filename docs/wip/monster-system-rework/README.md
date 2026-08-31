# Monster System Rework - Design Workspace

> Branch: `codex/mockups`  
> Status: Detailed mockup / implementation planning  
> Engine baseline: Unreal Engine 5.8  
> Purpose: 몬스터 시스템 리워크의 확정 방향, 실험 항목, 보류 항목을 분리하고 이후 Codex 구현의 기준점으로 사용한다.

## 1. 문서 목적

이 폴더는 PROJECT-MA의 Monster System Rework를 위한 임시 설계 워크스페이스다.

현재 목표는 "가장 많은 기능"을 만드는 것이 아니다. 목표는 다음 세 가지다.

1. 몬스터가 기존 MA Skill System을 자연스럽게 재사용한다.
2. 몬스터 종류를 직접 많이 복제하지 않아도, 공용 변주 규칙으로 높은 전투 다양성을 만든다.
3. 몬스터가 가진 능력과 실제 행동 성향을 분리하여 같은 구성에서도 다른 전투 경험을 만들 수 있게 한다.

이 문서는 브레인스토밍 결과를 그대로 확정하지 않는다. 이미 합의된 방향과 아직 검증이 필요한 부분을 상태 라벨로 구분한다.

## 2. 상태 라벨

- **Fact**: 현재 프로젝트 코드 또는 사용자 결정으로 확인된 사실.
- **Decision**: 현재 설계 방향으로 채택한 내용.
- **Candidate**: 유력하지만 구현 전 세부 검증이 필요한 내용.
- **Experiment**: 실제 샘플 구현/플레이 테스트 후 유지 여부를 결정할 내용.
- **Deferred**: 현재 리워크 범위에서는 구현하지 않는 내용.
- **Open Question**: 의도적으로 아직 결정하지 않은 내용.

Codex는 `Decision`을 임의로 바꾸지 않는다. 변경이 필요하면 이유와 영향을 먼저 기록한다.

## 3. 문서 구성

1. `01_CurrentStateAndGoals.md`
   - 현재 몬스터 시스템 구조, 문제점, 리워크 목표와 비목표
2. `02_SharedTraitTree.md`
   - Shared Trait Tree, Monster Set, Complexity, 랜덤 변주, Visual Telegraph
3. `03_IndividualBehavior.md`
   - 개인 성향치 7종과 AI 판단에 대한 의미
4. `04_AIAndSkillArchitecture.md`
   - Skill System / AI / StateTree / BT / EQS의 책임 경계
5. `05_ImplementationPlan.md`
   - 구현 순서, 검증 방법, 실패 조건, Codex 작업 규칙

## 4. 현재 핵심 방향

### 4.1 Monster Set

개발자가 직접 만드는 핵심 콘텐츠 단위는 몬스터 종류다.

예:

- Goblin
- Slime
- Golem
- Wolf

각 Monster Set은 자신의 기본 정체성과 사용할 Shared Trait Tree 목록을 가진다.

몬스터 타입 프리셋이 자동으로 Trait Tree를 정하지 않는다.

예:

```
Goblin Set
- Weapon Tree
- Body Tree
- Element Tree
- Armor Tree

Slime Set
- Body Tree
- Element Tree
- Mutation Tree
```

휴머노이드라고 반드시 무기를 쓰는 것도 아니고, 비휴머노이드라고 반드시 무기를 못 쓰는 것도 아니므로 Tree 사용 여부는 Monster Set이 직접 선택한다.

### 4.2 Shared Trait Tree

Trait Tree는 특정 몬스터에 종속되지 않는 공용 변주 데이터다.

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

한 경로를 선택하면 부모부터 선택 노드까지의 변화가 누적된다.

### 4.3 Final Monster

최종 몬스터는 하나의 변종 에셋이 아니라 여러 독립 Trait 축의 결과가 합쳐진 결과물이다.

```
Monster Base
+ Weapon Trait Result
+ Body Trait Result
+ Element Trait Result
+ Armor Trait Result
+ Individual Behavior
= Resolved Monster
```

### 4.4 스킬 랜덤 지급 금지

기본 랜덤성은 "스킬 자체를 임의 지급"하는 방식이 아니다.

```
Trait 선택
→ Trait가 Skill/Stat/Visual을 변경
→ 최종 Skill Set 형성
```

이렇게 해야 랜덤성이 규칙을 가지며 플레이어가 외형을 통해 능력을 학습할 수 있다.

### 4.5 개인성향

Trait는 "무엇을 가진 몬스터인가"를 만든다.

Individual Behavior는 "그 능력을 어떻게 사용하는가"를 만든다.

현재 개인성향 후보는 다음 7개다.

- Prediction
- Reaction
- Evasion
- Caution
- Initiative
- Persistence
- Preemptiveness

Group Behavior는 별도 계층으로 보고 현재는 보류한다.

## 5. 이번 리워크에서 하지 않는 것

### Deferred: Mass Entity

현재 MA의 몬스터는 Character, CharacterMovement, GAS, Skill System, 상태 시스템 등 Actor 중심 기능을 많이 사용한다. 대량 Entity 처리로 이전할 명확한 요구가 없다.

### Deferred: Smart Object

기본 전투 AI의 필수 요소가 아니다. 향후 포탑, 감시지점, 예약형 상호작용 등 실제 요구가 생길 때 다시 검토한다.

### Deferred: Group Behavior

무리 선동, 합류, 지원, 대형, 타겟 공유 등은 가치가 있지만 개인 AI가 먼저 완성된 이후 별도 시스템으로 확장한다.

### Out of scope: Spawn Rework

스폰 문제는 로컬 작업에서 별도로 해결된 상태다. Monster Rework에서 다시 설계하지 않는다.

## 6. 핵심 설계 원칙

1. 몬스터 전용 공격 구현을 늘리지 않는다.
2. 기존 MA Skill System을 실제 공격 실행 계층으로 사용한다.
3. AI는 "무엇을 할지" 결정하고 Skill System은 "그 행동을 실제로 실행"한다.
4. 몬스터 종류와 몬스터 변주를 분리한다.
5. Shared Trait Tree는 최초 사용 몬스터에 종속되지 않는다.
6. Monster Set이 사용할 Trait Tree를 직접 선택한다.
7. Trait Tree 각 축은 가능한 한 다른 축을 몰라도 적용 가능해야 한다.
8. 변주는 가능한 한 Visual Telegraph를 가져야 한다.
9. 랜덤성은 Skill 자체가 아니라 Trait 경로와 Complexity에 부여한다.
10. 개인성향은 Skill/Stat과 별도 계층으로 유지한다.
11. 현재 필요한 요구를 만족하는 가장 작은 구조를 우선한다.
12. 반드시 구현될 미래 요구는 현재 구조가 막지 않게 설계하되, 가능성만 있는 미래를 위한 추상화는 만들지 않는다.
