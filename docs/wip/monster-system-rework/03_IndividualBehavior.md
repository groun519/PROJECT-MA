# 03 - Individual Behavior Personality

## 1. 목적

Trait Tree가 "이 몬스터가 무엇을 가지고 있는가"를 결정한다면 Individual Behavior는 "그 능력을 어떻게 사용하는가"를 결정한다.

같은 Species, 같은 Trait, 같은 Skill Set을 가진 몬스터도 행동 성향이 다르면 실제 전투 체감이 달라질 수 있다.

현재 개인성향은 0.0 ~ 1.0 정규화 값을 기본 표현으로 고려한다.

값 자체가 확률을 의미하지는 않는다.

## 2. 현재 개인성향 7종

### Decision: Prediction

**정의:** 현재 관측 가능한 정보로 미래 상황을 얼마나 잘 예측하는가.

고려 가능한 정보 예:

- Player 현재 위치
- Player 이동 방향/속도
- 공격 선딜
- 투사체 진행 방향
- 최근 관측 정보

`Prediction = 0`

- 현재 상태 중심
- 현재 위치를 향한 단순 공격
- 실제 위협이 나타난 뒤 반응

`Prediction = 1`

- 현재 정보로 계산 가능한 범위에서 미래 위치를 적극 추정
- 이동하는 Target에 선행 사격 가능
- 공격 궤적/선딜을 이용해 위험 발생 전 위치 조정 가능

중요:

Prediction 1은 미래를 아는 것이 아니다.

Perception/관측으로 얻지 못한 정보를 읽어서는 안 된다.

```
Observed Information
→ Prediction
→ estimated future state
```

### Decision: Reaction

**정의:** 상황을 인지한 뒤 행동을 시작하기까지 얼마나 빠르게 반응하는가.

Prediction과 분리한다.

예:

```
Prediction 1.0
Reaction 0.2
```

위험을 정확히 예측해도 행동 시작이 느릴 수 있다.

반대로:

```
Prediction 0.2
Reaction 1.0
```

미리 읽지는 못하지만 실제 사건이 발생하면 즉시 반응할 수 있다.

Reaction은 단순 애니메이션 속도 배율이 아니라 AI Decision/Action 시작 지연을 조절하는 값으로 본다.

### Decision: Evasion

**정의:** 위험을 피할 때 얼마나 좋은 이동/회피 경로를 선택하는가.

가장 중요한 규칙:

> Evasion은 회피 확률이 아니다.

`Evasion = 0`

- 적극적인 회피를 거의 하지 않음
- 일반 Movement만 유지할 수 있음

`Evasion = 1`

- 현재 알고 있는 정보 안에서 가장 안전하고 유리한 회피 경로를 선택하려 함

고려 후보:

- 투사체 궤적
- 공격 범위
- 공격 도달 시점
- Player 위치
- 장애물
- NavMesh 접근 가능성
- 회피 후 남는 이동 공간
- 다른 위험 영역

Evasion이 높다고 무조건 모든 공격을 피하는 것은 아니다.

공간이 없거나 Reaction이 늦거나 Prediction이 실패하면 맞을 수 있다.

### Candidate: EQS 기반 회피 위치 평가

의사결정:

```
Threat detected
→ AI decides to evade
```

공간 문제:

```
EQS
→ candidate positions
→ safety scoring
→ best available route/point
```

즉 EQS는 "피할지"를 결정하지 않고 "어디로 피할지"를 해결한다.

### Decision: Caution

**정의:** 공격을 시작할 때 그 공격이 실제로 Target에 닿을 가능성을 얼마나 엄격하게 확인하는가.

이 값은 위험 회피 성향이 아니다.

`Caution = 0`

- 대략 사거리 안이면 공격 시도 가능
- Target 이동 때문에 빗나갈 가능성이 높아도 공격할 수 있음

`Caution = 1`

- 실제 타격 시점까지 고려하여 공격 성공 가능성이 충분할 때 Commit

고려 후보:

- 현재 거리
- Skill 실제 사거리
- Windup
- 타격 발생 시점
- Target Velocity
- Prediction 결과
- 공격 Area/Shape

개념:

```
Skill Candidate
+ current geometry
+ timing
+ predicted target state
→ expected hit confidence
→ Caution threshold
→ commit / reposition / wait
```

Caution과 Prediction은 독립적이다.

Prediction은 미래를 얼마나 잘 계산하는지,
Caution은 그 정보로 공격을 얼마나 엄격하게 확정하는지 결정한다.

### Decision: Initiative

**정의:** 전투가 시작된 후 공격 기회를 얼마나 적극적으로 만들어내는가.

낮음:

- 좋은 상황을 기다림
- 상대 행동에 반응하는 비율이 높음

높음:

- 접근
- 거리 조절
- 압박
- 공격 가능한 각 만들기
- Skill을 이용해 상황을 능동적으로 만듦

Preemptiveness와 분리한다.

Preemptiveness는 전투 시작 여부,
Initiative는 이미 시작된 전투의 주도성을 의미한다.

### Decision: Persistence

**정의:** 현재 목표/행동 목적을 얼마나 쉽게 포기하지 않는가.

적용 후보:

- Target 추격
- 선택한 공격 루트
- 원하는 위치 확보
- 특정 Player 압박
- 마지막으로 본 위치 탐색

낮음:

- 상황이 조금만 바뀌어도 다른 선택으로 전환

높음:

- 현재 목적을 더 오래 유지
- 실패한 접근/추격을 다시 시도

주의:

Persistence가 높다고 영원히 추격하면 안 된다.

Level/Encounter가 강제하는 하드 리미트는 Personality보다 우선한다.

### Decision: Preemptiveness

**정의:** 같은 Team이 아닌 대상을 상대로 얼마나 쉽게 먼저 교전을 시작하는가.

낮음:

- 비아군을 보아도 먼저 공격하지 않을 수 있음
- 공격받거나 특정 위협 조건이 필요

높음:

- 감지 가능한 비아군을 적극적으로 교전 대상으로 취급

개념:

```
Perception
→ relationship/team check
→ non-team actor
→ Preemptiveness + context
→ engage / ignore / observe
```

이 값은 개인성향이다.

다른 몬스터가 없어도 독립적으로 의미가 있기 때문이다.

## 3. 값들의 책임 구분

```
Preemptiveness
= 싸움을 시작할 것인가

Prediction
= 앞으로 무엇이 일어날지 얼마나 잘 읽는가

Reaction
= 인지한 뒤 얼마나 빨리 행동하는가

Initiative
= 전투를 얼마나 능동적으로 주도하는가

Caution
= 지금 공격하면 실제로 맞을지 얼마나 엄격히 판단하는가

Evasion
= 피해야 할 때 얼마나 좋은 회피 경로를 찾는가

Persistence
= 현재 목적을 얼마나 오래 유지하는가
```

서로 비슷한 감정 단어를 추가하기보다 실제 AI 알고리즘의 서로 다른 판단 지점에 대응하는 것을 우선한다.

## 4. 조합 예

### Example A - 영리하지만 소극적인 몬스터

```
Prediction     0.9
Reaction       0.9
Evasion        0.9
Caution        0.8
Initiative     0.2
Persistence    0.3
Preemptiveness 0.2
```

특징:

- 잘 읽고 잘 피한다.
- 맞지 않을 공격을 잘 던지지 않는다.
- 먼저 싸움을 걸거나 계속 압박하는 빈도는 낮다.

### Example B - 난폭한 추격자

```
Prediction     0.3
Reaction       0.9
Evasion        0.5
Caution        0.1
Initiative     1.0
Persistence    1.0
Preemptiveness 1.0
```

특징:

- 적을 보면 즉시 싸움을 시작한다.
- 전투 중 계속 압박한다.
- 공격 성공 여부를 엄격하게 계산하지 않아 헛공격이 나올 수 있다.
- 한번 정한 Target을 오래 추적한다.

### Example C - 정확하지만 느린 사수

```
Prediction     0.9
Reaction       0.3
Evasion        0.2
Caution        1.0
Initiative     0.4
Persistence    0.6
Preemptiveness 0.5
```

특징:

- 조준/예측 정확도가 높다.
- 확실한 공격만 사용한다.
- 반응과 회피가 느려 Player가 선제 압박하면 약점을 만들 수 있다.

## 5. Trait와 Personality의 관계

### Experiment: Base Personality + Trait Modifier

가능한 구조:

```
Base Personality
+ Trait Personality Modifiers
= Resolved Personality
```

예:

- Large Body → Evasion 감소 후보
- Speed 계열 → Evasion/Reaction 증가 후보
- Heavy 특성 → 특정 값 조정 후보

하지만 초기 Trait 시스템은 Personality를 필수로 수정하지 않는다.

Trait와 Personality가 지나치게 결합되면 각 축의 독립성이 깨진다.

먼저 Base Personality만으로 AI 차이를 검증하고, 실제로 자연스러운 Modifier가 필요한 Trait에만 추가한다.

## 6. Group Behavior

### Deferred

다음 개념은 개인성향과 분리한다.

- 주변 아군을 같은 Target으로 선동
- 아군 전투에 합류
- 개인 전투 선호
- 아군 지원 선호
- 대형 유지
- 포위
- 타겟 분산/공유
- 역할 분담

Group Behavior는 다른 Actor와의 관계가 있어야 의미가 생기므로 Individual Behavior와 다른 계층으로 본다.

개인성향 시스템이 실제 전투에서 충분히 작동한 이후 별도 문서/시스템으로 확장한다.

## 7. 구현 원칙

1. Personality 값은 직접적인 성공/실패 확률보다 판단 기준과 품질을 변화시킨다.
2. Prediction은 비공개 정보를 읽지 않는다.
3. Evasion 1.0도 물리적으로 불가능한 공격을 자동 회피하지 않는다.
4. Caution 1.0도 완벽한 미래 지식을 의미하지 않는다.
5. Personality는 Game Rule의 하드 제한보다 우선하지 않는다.
6. 값 하나가 다른 값의 책임을 대신하지 않게 한다.
7. 초기에는 7개 이상 늘리지 않는다. 새 값은 기존 값으로 표현할 수 없는 실제 행동 요구가 생겼을 때만 추가한다.
