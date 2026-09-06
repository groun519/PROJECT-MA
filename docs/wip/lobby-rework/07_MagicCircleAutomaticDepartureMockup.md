# 07. Magic Circle Automatic Departure Mockup

## 목적

다음 작업에서 Hub의 `AMAMagicCircle`을 실제 Space Transition 출발 진입점으로 연결한다.

현재 Persistent Space Transition의 `Load -> Close -> Move -> Open -> Unload` 흐름은 유지하고, 이번 작업에서는 **전원이 마법진 안에 일정 시간 머물렀을 때 자동으로 출발 요청이 발생하는 흐름**만 추가한다.

## 기본 동작

```text
Player enters Magic Circle
-> PlayersInCircle 갱신
-> 현재 참가 Player 전원이 Circle 안인지 검사
-> 전원 진입 시 3초 대기 시작
-> 3초 동안 전원이 계속 Circle 안에 있으면 Departure 확정
-> 기존 SpaceTransition 진입
```

### 이탈

3초가 끝나기 전에 한 명이라도 Circle 밖으로 나가면 대기를 즉시 취소한다.

```text
3 / 3 진입
-> 3초 대기 시작
-> 1.7초에 한 명 이탈
-> 대기 취소 / 초기화

다시 3 / 3 진입
-> 새로운 3초 대기 시작
```

부분 진행 시간을 유지하지 않는다.

## UI

현재 몇 명이 Circle에 들어왔는지 표시하는 UI는 만들지 않는다.

다음과 같은 숫자 표시는 사용하지 않는다.

```text
1 / 3
2 / 3
3 / 3
```

이번 작업 범위에서는 별도 Countdown UI도 만들지 않는다.

마법진 자체의 발광, 회전, 사운드 같은 월드 연출은 필요가 생기면 별도로 추가한다.

## 책임 경계

### `AMAMagicCircle`

- Circle 내부 Player 감지와 목록 관리
- 전원 진입 여부 판단
- 전원 진입 후 3초 유지 판정
- 중간 이탈 시 대기 취소
- 출발 조건이 성립했다는 의미 전달

Magic Circle은 다음을 직접 소유하지 않는다.

- Destination map 선택
- Streaming
- Client Ready 수집
- Mask Close / Open
- Player Handoff
- Source Unload

즉 Magic Circle이 직접 전환 절차를 조립하지 않는다.

```text
MagicCircle
-> Departure 조건 성립

SpaceTransition 쪽 기존 진입점
-> 실제 Destination 준비 및 전환
```

### `UMASpaceTransitionSubsystem`

현재 책임을 그대로 유지한다.

```text
RequestTransition
-> Load Destination
-> All Clients Ready
-> Close
-> Move Players
-> Open
-> Unload Source
-> Promote Destination
```

이번 작업은 이 내부 흐름을 다시 설계하지 않는다.

## 이번 작업에서 하지 않을 것

다음은 자동 Departure가 동작한 뒤 별도 작업으로 연결한다.

- Transition Restricted
- 전환 중 고정 저속
- 일반 이동속도 Buff / Debuff 무시
- Safe Radius 밖 이탈 방지
- Skill 차단
- Dash / Jump / Teleport 차단
- Knockback 제한
- Interaction 차단
- Departure 확정 후 Ready 취소 금지
- Handoff 순간 Full Lock
- Countdown UI
- 실제 Battle generation
- `LevelManager`의 `BattleReady`를 Destination 준비 조건에 연결

## 다음 단계에서의 최종 흐름

이번 작업 완료 후 다음 고도화는 아래 흐름을 목표로 한다.

```text
전원 Circle 진입
-> 3초 유지
-> Departure 확정
-> Transition Restricted
-> Destination Ready
-> Close
-> Handoff
-> Open
-> Restricted 해제
```

이번 작업에서는 `Transition Restricted` 전까지만 연결한다.

## 완료 기준

- 1인 플레이에서는 혼자 Circle에 올라간 뒤 3초 후 기존 Persistent Space Transition이 시작된다.
- 멀티플레이에서는 현재 참가 Player 전원이 Circle 안에 있어야 3초 대기가 시작된다.
- 3초 안에 한 명이라도 이탈하면 출발이 취소된다.
- 다시 전원이 들어오면 3초를 처음부터 다시 센다.
- Player 수를 표시하는 UI는 없다.
- Magic Circle이 Destination map이나 Streaming/Mask/Handoff 구현을 직접 소유하지 않는다.
- 기존 `UMASpaceTransitionSubsystem`의 전환 흐름을 불필요하게 변경하지 않는다.
