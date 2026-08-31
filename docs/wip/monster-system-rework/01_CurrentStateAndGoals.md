# 01 - Current State and Rework Goals

## 1. 현재 프로젝트 사실

### Fact: 공통 Character/GAS 기반

`AMonster`는 `AMACharacter`를 기반으로 하며 프로젝트 공통 Ability/Attribute/Status/Skill 계층을 이미 공유한다.

현재 구조에서 몬스터는 별도의 완전히 독립된 전투 프레임워크가 아니라 MA의 공통 Character + GAS 기반 위에 존재한다.

### Fact: Monster는 이미 MA Skill System을 사용한다

현재 `AMonster`에는 Monster Skill Slot 데이터와 Skill 초기화/선택 경로가 존재하며 `UMASkillManagerComponent`와 GAS를 통해 Skill을 실행할 수 있다.

따라서 이번 리워크의 출발점은 "몬스터용 새 스킬 시스템을 만든다"가 아니다.

목표는 기존 Skill System을 몬스터가 더 자연스럽고 강력하게 사용할 수 있도록 AI/변주 구조를 재정리하는 것이다.

### Fact: 현재 AI Brain은 Behavior Tree

`AMAAIController`는 Behavior Tree를 실행하며 AI Perception을 통해 대상을 탐지한다.

현재 Perception은 적대 Actor를 수집하고 가까운 Actor를 Target으로 선택하는 단순한 정책을 가진다.

### Fact: 현재 StateTree는 Primary Brain이 아니다

현재 StateTree는 Monster Pattern 선택을 위해 BT Task 내부에서 임시 실행된다.

대략적인 흐름:

```
Behavior Tree Task
→ temporary StateTree execution
→ active State names
→ Pattern DataTable RowName
→ PatternPlan
→ Skill Module 교체/선택
→ BT movement
→ Skill execution
```

2026-06-21 Monster Refactoring 커밋의 의도 역시 "StateTree 기반 Monster Pattern 선택"이다.

즉 StateTree는 현재 전체 AI 상태를 운영하는 Brain이 아니라 Pattern Planner/Selector로 사용되고 있다.

### Fact: State 이름과 DataTable RowName이 암묵적으로 결합되어 있다

현재 Pattern State 이름이 PatternData의 RowName과 대응한다.

이 방식은 이름 변경이 컴파일 타임에 검증되지 않고, StateTree와 DataTable 사이의 결합이 숨겨진다.

### Fact: 현재 AI Skill 실행은 Player Input 흐름을 일부 모방한다

BT Skill 실행 Task는 Slot Input ID를 얻고 Ability Input Press/Release 흐름과 `TryActivateSkill`을 함께 사용한다.

리워크 시 "AI가 Player Input을 흉내 내야 하는가"를 재검토해야 한다.

### Fact: Monster Movement는 기본 Character/NavMesh 계층에 가깝다

현재 전용 Movement Component의 핵심 차이는 Nav path에서 acceleration을 사용하는 정도다.

고지대 Drop/Jump 등은 별도 Level Rework의 NavLink/Traversal 설계와 연결해야 한다.

### Fact: Spawn Rework는 이번 범위에서 제외

사용자 로컬 작업에서 스폰 문제는 해결된 상태다.

현재 repository의 이전 WaveManager/Pool 구조가 보이더라도 Monster Rework가 Spawn 설계를 다시 소유하지 않는다.

## 2. 문제 정의

현재 시스템의 가장 큰 문제는 "기능 부족"보다 책임 경계가 불명확한 점이다.

### 문제 A - AI Pattern과 Skill Composition의 결합

현재 Pattern Data가 State 이름, DataTable Row, Skill Module 구성까지 연결한다.

AI가 어떤 행동을 선택하는지와 그 Skill이 어떻게 조립되는지가 너무 가까이 결합되어 있다.

### 문제 B - 몬스터 콘텐츠 확장 비용

몬스터가 복잡해질수록 각 몬스터마다 별도 Skill Set/Pattern/Variant를 수작업으로 늘리면 제작 비용이 선형 이상으로 증가한다.

MA는 몬스터 전투 자체가 어느 정도 복잡해야 게임의 맛이 살아나는 방향을 목표로 한다.

따라서 "종 하나 = 패턴 하나"의 단순 구조보다 재사용 가능한 변주 축이 필요하다.

### 문제 C - 랜덤 Skill 지급은 가독성이 낮다

스킬을 직접 랜덤 지급하면 같은 외형의 몬스터가 예측하기 어려운 행동을 할 수 있다.

이는 로그라이크 변주를 만들 수 있지만 학습성과 Visual Telegraph를 약화시킨다.

### 문제 D - AI 능력과 AI 성향이 분리되어 있지 않다

같은 Skill Set을 가진 몬스터가 모두 같은 판단 품질과 공격 타이밍을 가지면 전투 다양성이 제한된다.

Skill/Stat이 "가능한 행동"이라면 AI Personality는 "그 행동을 사용하는 방식"이어야 한다.

### 문제 E - 현재 Target 개념이 Player 중심 교전에 가깝다

향후 Goblin, Golem처럼 서로 다른 Team의 몬스터가 같은 필드에 존재할 수 있다.

AI가 단순히 Player만 적으로 취급하거나 "다른 Team = 무조건 적대"로 고정되면 다음 플레이를 만들기 어렵다.

- 서로 다른 몬스터 종족끼리 자연스럽게 싸우기
- Player가 서로 다른 몬스터 집단의 충돌을 유도하기
- 비아군이지만 선공하지 않는 몬스터 만들기
- Team Damage Rate에 따라 같은 Team의 공격 위험을 다르게 평가하기

따라서 Team 판정과 실제 공격 의지를 분리해야 한다.

## 3. 리워크 목표

### Decision: 복잡한 몬스터를 만들기 쉬워야 한다

MA의 몬스터는 단순 추격 + 랜덤 공격만 반복하는 구조를 목표로 하지 않는다.

다음과 같은 복잡성을 지원할 수 있어야 한다.

- 여러 공격 기술
- 거리/상황별 Skill 선택
- 공격 연계
- 회피와 위치 선택
- 실제 공격 성공 가능성을 고려한 공격 타이밍
- 플레이어 움직임 예측
- 개체별 행동 성향 차이
- Trait에 따른 Skill/Stat/Visual 변주

### Decision: 몬스터도 공용 Skill System을 사용한다

공격, 투사체, 범위기, 버프/디버프 등 일반 전투 기능은 가능한 한 기존 MA Skill System으로 표현한다.

AI 전용 Attack 함수 증식을 피한다.

### Decision: Monster Species와 Variation을 분리한다

개발자는 핵심적으로 Monster Species/Set을 만든다.

Variation은 공용 Shared Trait Tree 조합으로 만든다.

### Decision: 플레이어가 Variation을 읽을 수 있어야 한다

무기 변화는 Weapon Mesh, 속성 변화는 Material/VFX, 체형 변화는 크기/실루엣 등 가능한 한 Gameplay Telegraph를 가진다.

### Decision: Team은 단순한 Ally / Non-Team 기반으로 사용한다

Monster Set은 기본 Team을 가질 수 있다.

예:

```
Goblin Set → Team 2
Golem Set  → Team 3
```

같은 Team은 Ally다.

다른 Team은 Non-Team이며, 즉시 Enemy로 고정하지 않는다.

Non-Team에 대한 선제 공격 여부는 Individual Behavior의 `Preemptiveness`가 결정한다.

### Decision: Monster끼리도 동일한 전투 규칙으로 싸울 수 있어야 한다

다른 Team의 Monster가 서로 Target이 될 수 있어야 하며, Monster 전용 가짜 전투 연출이 아니라 실제 MA Skill/Damage 경로를 사용한다.

이를 통해 Player가 서로 다른 Monster 집단을 충돌시키는 상황도 자연스럽게 만들어질 수 있다.

### Decision: Team Damage Rate는 Match/Map Rule로 조절 가능하게 한다

같은 Team에게 들어가는 Damage 배율을 0~100% 범위에서 설정할 수 있게 하는 방향을 채택한다.

Player와 Monster 모두 같은 규칙을 적용받는 것을 목표로 한다.

정확한 설정 소유 클래스와 기본값은 구현 전 현재 Match/Map Setting 구조를 확인한 뒤 정한다.

## 4. 비목표

### Deferred: 수백~수천 Actor를 위한 Mass 전환

현재 요구가 아니다.

### Deferred: 모든 몬스터를 하나의 완전 범용 AI로 강제

공용 기반은 최대한 재사용하되, Boss나 특수 몬스터가 전용 행동 구조를 필요로 할 가능성은 열어둔다.

### Deferred: 모든 Trait 간 Synergy 규칙

초기에는 Trait 축끼리 최대한 독립적으로 누적되어야 한다.

`Fire + Sword일 때 특별 규칙`, `Large + Fire + Sword일 때 또 다른 규칙` 같은 조합 전용 예외가 늘어나면 조합 시스템의 장점이 사라진다.

### Deferred: Group AI

개인성향 완성 후 확장한다.

## 5. 성공 조건

리워크의 첫 성공 기준은 "시스템이 거대하다"가 아니다.

다음이 가능하면 기반 성공으로 본다.

1. 동일한 Goblin Set이 서로 다른 Trait 조합으로 의미 있게 다른 Skill/Stat/Visual을 가진다.
2. 동일한 Trait Tree가 둘 이상의 Monster Set에서 재사용된다.
3. 같은 Skill 구성의 몬스터도 Personality 값에 따라 전투 방식이 달라진다.
4. AI가 Skill의 내부 구현을 직접 알 필요 없이 Skill System을 실행한다.
5. Trait를 추가하기 위해 기존 몬스터 C++ 클래스를 수정할 필요가 없다.
6. 플레이어가 최소한 주요 Trait를 외형으로 구분할 수 있다.
7. Goblin과 Golem처럼 다른 Team의 Monster가 별도 특수 코드 없이 서로를 정상적인 Combat Target으로 취급할 수 있다.
8. 같은 Team과 다른 Team Damage가 하나의 중앙 규칙으로 일관되게 계산된다.
9. `Preemptiveness` 값 차이만으로 Non-Team을 먼저 공격하는 몬스터와 먼저 공격하지 않는 몬스터를 구분할 수 있다.
