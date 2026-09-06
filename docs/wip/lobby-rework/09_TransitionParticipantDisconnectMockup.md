# 09. Transition Participant / Disconnect Mockup

## 목적

Persistent Space Transition 중 플레이어가 접속 종료해도 남은 플레이어의 전환이 멈추지 않도록 참가자와 단계별 응답 대기를 명확히 관리한다.

이 문서는 플레이어 수에 따른 맵 생성, 몬스터 스케일링, 런타임 난이도 조정까지 다루지 않는다. 이 문서의 범위는 오직 **이번 전환에 누가 참여하는지**와 **전환 도중 참가자가 사라졌을 때 Pending 대기를 어떻게 닫는지**이다.

---

## 현재 확인된 문제

현재 Space Transition은 각 단계에서 서버가 클라이언트 응답을 기다리기 위해 `PendingPlayers`를 사용한다.

정상 흐름은 다음과 같다.

```text
Participants = [Host, A, B]

Loading Pending = [A, B]
A Loaded -> Pending = [B]
B Loaded -> Pending = []
-> Closing 진행
```

하지만 B가 응답하기 전에 연결을 종료하면 B는 더 이상 응답할 수 없다.

```text
Loading Pending = [B]
B Disconnect
-> B의 응답은 영원히 오지 않음
-> Pending이 비지 않음
-> Transition 정지
```

같은 문제가 Loading뿐 아니라 Closing / Opening의 단계별 응답 대기에서도 발생할 수 있다.

이 문제를 Weak Pointer가 언젠가 무효화되기를 기다리는 방식으로 해결하지 않는다. 서버가 플레이어 이탈을 확정한 시점에 전환 상태도 즉시 정리되어야 한다.

---

# 핵심 결정

## 1. 이번 전환 참가자는 Magic Circle이 출발 확정 순간 결정한다

현재 세션 전체 플레이어 목록과 이번 전환의 참가자 목록을 같은 개념으로 취급하지 않는다.

Magic Circle은 이미 다음 의미를 소유한다.

- 현재 Circle 내부에 누가 있는가
- 현재 플레이어 전원이 Circle 내부에 있는가
- 전원 진입 상태가 3초 동안 유지되었는가
- 출발 조건이 최종 확정되었는가

따라서 3초 유지 조건이 완료되고 출발이 확정되는 순간, Magic Circle이 현재 내부의 플레이어를 이번 전환의 참가자로 확정해 Transition에 넘긴다.

```text
MagicCircle
-> PlayersInside 관리
-> 전원 진입 확인
-> 3초 유지
-> 재검증 성공
-> Departure 확정
-> TransitionParticipants snapshot 전달
```

이 결정은 코드가 게임 규칙과 같은 의미로 읽히게 한다.

> "마법진 안에 있던 플레이어들이 함께 출발한다."

### 금지

Magic Circle이 다음 역할까지 가져가면 안 된다.

- Destination 로딩
- Pending 응답 관리
- Close / Open 진행
- Player Handoff
- Transition Abort

Magic Circle의 책임은 **출발 조건과 출발 참가자 확정까지**이다.

---

## 2. GameState PlayerArray는 세션 플레이어 기준이며 Transition 참가자 목록을 대체하지 않는다

`GameState->PlayerArray`는 현재 게임 세션에 존재하는 플레이어의 권위 있는 기준으로 사용한다.

그러나 전환이 시작된 뒤 새 플레이어가 세션에 들어올 수 있으므로 이번 전환 참가자와 항상 같다고 가정하지 않는다.

예:

```text
Departure 확정 시
Participants = [Host, A, B]

그 직후 C Join
GameState.PlayerArray = [Host, A, B, C]
Participants = [Host, A, B]
```

C는 현재 진행 중인 전환에 중간 삽입하지 않는다.

따라서 의미를 분리한다.

```text
GameState.PlayerArray
= 현재 세션에 존재하는 전체 플레이어

TransitionParticipants
= 이번 Departure에서 출발이 확정된 플레이어
```

별도의 전역 Player Manager나 플레이어 레지스트리를 새로 만들 필요는 없다.

---

## 3. PendingPlayers는 유지한다

`TransitionParticipants`와 `PendingPlayers`는 목적이 다르므로 둘 다 존재할 수 있다.

```text
TransitionParticipants
= 이번 전환에 참여하는 전체 대상

PendingPlayers
= 현재 Phase에서 아직 완료 응답을 보내지 않은 대상
```

예:

```text
Participants = [Host, A, B]
Pending = [A, B]

A 완료
Participants = [Host, A, B]
Pending = [B]
```

즉 Pending은 참가자 배열의 중복 캐시가 아니라 **단계별 barrier 상태**이다.

현재 구현의 Pending 기반 흐름을 불필요하게 뒤집지 않는다.

---

## 4. Client Disconnect는 남은 플레이어의 Transition을 Abort하지 않는다

전환 참가자 한 명이 접속 종료하면 그 플레이어만 이번 전환에서 제거한다.

```text
Participant Disconnect
-> TransitionParticipants에서 제거
-> PendingPlayers에서 제거
-> 현재 Phase가 진행 가능한지 즉시 재검사
-> 남은 참가자는 계속 전환
```

Loading / Closing / Opening 모두 같은 원칙을 사용한다.

### 예시 A - Loading 중 Disconnect

```text
Participants = [Host, A, B]
Pending = [A, B]

A Loaded
Pending = [B]

B Disconnect
Participants = [Host, A]
Pending = []

-> Loading barrier 완료
-> Closing 진행
```

### 예시 B - Closing 중 Disconnect

```text
Participants = [Host, A, B]
Closing Pending = [A, B]

A Closed
Pending = [B]

B Disconnect
Pending = []

-> Handoff / Open 진행
```

### 예시 C - Opening 중 Disconnect

```text
Participants = [Host, A, B]
Opening Pending = [A, B]

B Disconnect
Participants = [Host, A]
Pending = [A]

A Open 완료
Pending = []

-> Transition Finish
```

플레이어 한 명의 연결 종료가 다른 플레이어들의 정상 공간 전환을 취소하지 않는다.

---

## 5. Host Disconnect는 Transition 복구 대상이 아니다

현재 멀티플레이 구조는 Listen Server를 기준으로 한다.

Host가 게임을 종료하면 세션과 해당 World 자체가 종료되는 문제이므로 `UMASpaceTransitionSubsystem`이 Host 종료를 복구하거나 남은 참가자를 위한 전환을 계속하려고 하지 않는다.

```text
Host Disconnect
-> Listen Server / Session 종료 영역
-> Transition 복구 정책 범위 밖
```

따라서 `Participants == 0` 같은 별도 게임 정책을 Transition에 추가하지 않는다.

참가자가 모두 사라지는 상황은 Listen Server의 Host 종료와 함께 세션 생명주기가 끝나는 문제로 본다.

---

## 6. 전환 중 새 Join은 현재 전환에 포함하지 않는다

Departure 확정 이후 접속한 플레이어를 현재 진행 중인 Transition barrier에 추가하지 않는다.

```text
Departure 확정
Participants = [Host, A]

Loading 중 B Join
-> Participants 변화 없음
-> Pending 변화 없음
```

새 Join의 Spawn / 현재 Active Space 배치 정책은 별도의 Join-in-Progress 문제이며 이 목업에서 구현하지 않는다.

중요한 것은 새 플레이어가 들어왔다고 현재 Transition의 Pending 조건이 뒤늦게 늘어나지 않는 것이다.

---

# 소유권

## AMAMagicCircle

소유:

- Circle 내부 플레이어 인식
- 전원 Circle 진입 조건
- 3초 유지 조건
- 3초 완료 시 최종 재검증
- Departure 확정 시 참가자 snapshot 제공

소유하지 않음:

- Transition Phase
- 단계별 Pending
- Disconnect 이후 전환 진행 판단
- Destination 로딩 / Handoff / Open

---

## UMASpaceTransitionSubsystem

소유:

- 이번 전환의 `TransitionParticipants`
- 각 Phase의 `PendingPlayers`
- Loading / Closing / Opening barrier 진행
- 참가자 Disconnect 반영
- Pending 제거 후 즉시 다음 단계 진행 가능 여부 판단
- 기존 Abort / Finish / Promote 흐름

Subsystem은 세션 전체 플레이어 목록의 주인이 아니다.

---

## GameMode / GameState

`GameState->PlayerArray`는 현재 세션 플레이어의 권위 있는 목록으로 유지한다.

서버에서 Join / Logout을 확정하는 진입점은 GameMode 생명주기를 사용한다.

Disconnect 시 Transition에 전달해야 하는 의미는 단순하다.

```text
"이 Player가 세션에서 나갔다."
```

Transition은 그 Player가 현재 참가자인지, 현재 Pending인지 스스로 판단하고 자기 상태만 정리한다.

GameMode가 Transition Phase 정책을 구현하거나 Pending을 직접 수정하지 않는다.

개념 흐름:

```text
GameMode Logout
-> SpaceTransitionSubsystem::HandlePlayerDisconnected(Player)

SpaceTransitionSubsystem
-> 참가자인가?
   -> 아니면 무시
   -> 맞으면 Participants 제거
-> Pending에 있으면 제거
-> 현재 Phase barrier 재검사
```

정확한 함수명은 구현 시 기존 코드 네이밍에 맞춘다.

---

# 참가자 식별 기준

구현 시 참가자를 어떤 UObject 타입으로 저장할지는 현재 Transition RPC 구조를 우선한다.

현재 단계 응답과 RPC 대상이 `APlayerController` 중심이라면 억지로 모든 내부 상태를 `APlayerState`로 변환할 필요는 없다.

반대로 Magic Circle의 출발 참가자 인식이 PlayerState 중심이라면 전달 경계에서 필요한 Controller를 해석할 수 있다.

원칙은 다음 하나다.

> 참가자 식별 타입 통일을 목적으로 기존 RPC / Player lifecycle 구조를 대규모 리팩터링하지 않는다.

단, Disconnect 처리에서 이미 파괴 중인 객체를 오래 보관하는 구조는 피하고 기존 Weak Pointer 정책을 유지할 수 있다.

---

# Phase별 동작

## Idle

Disconnect가 발생해도 Transition이 진행 중이 아니므로 아무 작업도 하지 않는다.

---

## Loading

```text
Disconnect
-> Participants 제거
-> Pending 제거
-> Pending == 0인지 재검사
-> Destination load 자체가 준비되었는지도 기존 조건대로 확인
-> 모두 만족하면 TryBeginClose
```

Player가 빠졌다는 이유만으로 Destination load를 취소하지 않는다.

---

## Closing

```text
Disconnect
-> Participants 제거
-> Pending 제거
-> Pending == 0이면 기존 Closing 완료 경로 진행
```

Disconnected Player의 Close 완료 응답을 더 이상 기다리지 않는다.

---

## Handoff

서버가 실제로 Pawn들을 Destination으로 옮길 때는 현재 유효한 참가자만 대상으로 한다.

Disconnect되어 참가자에서 제거된 Player를 이동 대상으로 다시 찾거나 복구하려 하지 않는다.

---

## Opening

```text
Disconnect
-> Participants 제거
-> Pending 제거
-> Pending == 0이면 FinishTransition
```

Destination으로 이미 넘어간 다른 플레이어는 계속 Open한다.

---

# Pending barrier 공통화

Disconnect 처리 후 각 Phase마다 서로 다른 임의 분기 코드를 복제하지 않는다.

가능하면 기존 단계 진행 함수의 의미를 재사용한다.

예:

```text
HandleParticipantRemoved
-> Participants.Remove(Player)
-> PendingPlayers.Remove(Player)
-> EvaluateCurrentPhaseBarrier()
```

`EvaluateCurrentPhaseBarrier()` 같은 이름은 예시일 뿐이다.

목표는 다음이다.

- Loading barrier 완료 조건은 기존 Loading 완료 경로 한 곳
- Closing barrier 완료 조건은 기존 Closing 완료 경로 한 곳
- Opening barrier 완료 조건은 기존 Opening 완료 경로 한 곳

Disconnect가 별도의 두 번째 Transition state machine을 만들지 않아야 한다.

---

# Weak Pointer 정리 정책

명시적 Logout / Disconnect 이벤트가 주 경로다.

그러나 Pending 검사 시 이미 Invalid가 된 Weak Pointer를 방어적으로 제거하는 것은 허용한다.

```text
주 경로
= 명시적 Player Disconnect 이벤트

안전장치
= barrier 검사 시 invalid weak entry 제거
```

금지:

- Tick으로 매 프레임 PlayerArray와 Pending을 비교
- 일정 시간마다 전체 참가자를 Polling
- Weak Pointer가 자동으로 invalid 될 때까지 기다린 뒤 우연히 진행

Disconnect 직후 의미 있는 상태 변경이 발생하므로 이벤트 기반으로 처리한다.

---

# Transition Restricted와의 관계

`08_TransitionRestrictedMockup.md`의 제한 정책과 충돌하지 않는다.

Disconnected Player는 더 이상 복구 대상이 아니다.

남은 참가자들의 Restricted 상태는 기존 Transition Phase에 따라 그대로 유지한다.

```text
Departure 확정
-> Restricted 시작

한 Client Disconnect
-> 해당 Client만 참가자 제거
-> 남은 Player Restricted 유지
-> Transition 계속

Destination Open 시작
-> 남은 Player Restricted 해제
```

Disconnect가 발생했다고 남은 플레이어의 Restricted를 해제하거나 Transition을 Abort하지 않는다.

---

# PlayerCount / Monster Scaling과의 관계

이 목업에서는 PlayerCount가 Map topology, Map size, Voronoi, Path, Floor 생성에 영향을 주지 않는 것으로 전제한다.

PlayerCount 변화에 따라 몬스터 스탯 / 배율 / 이미 스폰된 몬스터를 어떻게 조정할지는 별도 Runtime Player Scaling 목업에서 다룬다.

따라서 Transition에서는 다음을 하지 않는다.

- PlayerCount 변경 때문에 Destination 재생성
- Disconnect 때문에 MapSeed 변경
- Disconnect 때문에 GenerationSettings reroll
- Disconnect 때문에 현재 전환 Abort
- 몬스터 HP / Damage / Spawn 수 직접 수정

Transition의 책임은 참가자 barrier 정리까지다.

---

# 구현 순서 제안

## Step 1. Magic Circle Departure payload 확장

현재 3초 완료 후 Departure 알림에서, 출발 확정 순간 Circle 내부의 현재 Player snapshot을 Transition 요청 경계까지 전달할 수 있게 한다.

이미 존재하는 `OnAllPlayersReady` / `RequestTransition` 연결을 불필요하게 재설계하지 말고 가장 짧은 경로로 참가자 의미만 추가한다.

목표:

```text
Departure request
= Destination + GenerationSeed + Participants
```

정확한 API 형태는 현재 코드 구조에 맞춘다.

---

## Step 2. TransitionParticipants 저장

Transition이 성공적으로 시작될 때 이번 전환의 참가자 snapshot을 저장한다.

- Begin / Request 실패 시 저장하지 않거나 즉시 Clear
- Finish 시 Clear
- Abort 시 Clear
- 다음 Transition에 이전 참가자가 남지 않음

---

## Step 3. Pending 구성 대상을 Participants로 제한

각 Phase에서 `GetWorld()`의 모든 PlayerController를 다시 긁어 Pending을 구성하지 않는다.

이번 전환에 확정된 `TransitionParticipants` 중 현재 유효한 원격 참가자를 기준으로 Pending을 구성한다.

Listen Server의 local host 처리 방식은 기존 동기 경로를 보존한다.

---

## Step 4. 공통 Player Logout 전달

공통 `AMAGameMode`의 서버 Logout 경계에서 SpaceTransitionSubsystem에 Player 이탈 사실을 전달한다.

Lobby 전용 slot 정리 같은 기존 `ALobbyGameMode::Logout()` 책임과 충돌하지 않도록 상속 호출 순서를 보존한다.

필요하다면 base `AMAGameMode::Logout()`에서 Transition 통지만 처리하고 LobbyGameMode는 기존 Lobby 상태 정리를 유지한다.

---

## Step 5. Participant removal + barrier 재검사

TransitionSubsystem에서 한 곳으로 닫는다.

```text
HandlePlayerDisconnected
-> TransitionParticipants.Remove
-> PendingPlayers.Remove
-> Current Phase barrier 재검사
```

Disconnect 전용으로 Loading / Closing / Opening의 로직 전체를 복제하지 않는다.

---

## Step 6. Reset 경로 검수

다음 모든 경로에서 Participants / Pending이 남지 않는지 확인한다.

- normal Finish
- Abort
- Destination load failure
- client preparation failure
- world shutdown
- 새 Transition 시작 전

---

# Acceptance Cases

## Case A - Loading 중 remote client 종료

```text
Host + A + B
-> Departure 확정
-> Loading
-> A ready
-> B disconnect
-> B Pending 제거
-> Host/A 기준 Closing 진행
-> Handoff
-> Open
-> Finish
```

기대 결과:

- 서버 무한 대기 없음
- A 전환 성공
- Destination 정상 Promote

---

## Case B - Closing 중 remote client 종료

```text
Host + A + B
-> 모두 Loading 완료
-> Closing
-> A Closed
-> B disconnect
-> Pending 종료
-> Handoff 진행
```

기대 결과:

- B 응답을 영원히 기다리지 않음
- 남은 참가자 정상 이동

---

## Case C - Opening 중 remote client 종료

```text
Host + A + B
-> Handoff 완료
-> Opening
-> B disconnect
-> A Open 완료
-> Finish
```

기대 결과:

- Destination을 다시 Abort하지 않음
- Source가 정상 unload됨
- Current Space가 Destination으로 Promote됨

---

## Case D - Departure 직후 새 client Join

```text
Departure snapshot = [Host, A]
-> Loading 시작
-> B Join
```

기대 결과:

- B가 현재 Pending에 추가되지 않음
- Host/A의 Transition barrier가 B 때문에 변하지 않음
- B의 실제 배치 정책은 별도 Join-in-Progress 시스템 범위

---

## Case E - Disconnect 대상이 현재 Participant가 아님

```text
Current Participants = [Host, A]
B disconnect
```

기대 결과:

- Transition 상태 변화 없음
- Pending 변화 없음
- 로그 스팸 없음

---

## Case F - 정상 전환

Disconnect가 없는 기존 Standalone / Listen Server + Client 전환이 이전과 동일하게 작동해야 한다.

이 기능 때문에 정상 전환 경로가 더 느려지거나 추가 Tick / Polling을 사용하면 안 된다.

---

# 완료 조건

다음을 모두 만족하면 이 작업을 완료로 본다.

- Magic Circle 출발 확정 순간 이번 전환 참가자를 명확히 고정한다.
- 새 Join이 진행 중 Transition에 자동 추가되지 않는다.
- Disconnect된 참가자를 더 이상 어떤 Phase에서도 기다리지 않는다.
- Loading / Closing / Opening 중 Disconnect 모두 남은 참가자가 정상 진행한다.
- Host 종료를 Transition 복구 문제로 만들지 않는다.
- GameState PlayerArray와 TransitionParticipants의 의미가 섞이지 않는다.
- PendingPlayers는 단계별 barrier 역할만 가진다.
- Disconnect 처리를 위한 Tick / polling / 별도 Player registry를 만들지 않는다.
- Finish / Abort 후 참가자와 Pending 상태가 완전히 비워진다.
- 기존 Transition 정상 경로를 보존한다.

---

# Codex 구현 지시 요약

이 목업을 구현할 때 범위를 확대하지 않는다.

```text
Implement transition participant snapshot and disconnect-safe pending barriers only.

- Magic Circle fixes the departure participant snapshot when the 3-second departure condition completes.
- SpaceTransitionSubsystem owns TransitionParticipants and per-phase PendingPlayers.
- New joins after departure are not added to the active transition.
- Remote client disconnect removes that participant from Participants/Pending and immediately re-evaluates the current phase barrier.
- Do not abort the transition because one client disconnected.
- Host shutdown is outside transition recovery scope.
- Use explicit server Logout/disconnect notification as the main path; weak-pointer cleanup is only defensive.
- Preserve the existing Loading/Closing/Handoff/Opening state machine and listen-server local synchronous behavior.
- Do not implement PlayerCount monster scaling, join-in-progress spawn policy, map regeneration, or a new generic player manager.
- Stop after this scope is complete and verified.
```
