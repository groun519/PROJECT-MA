# 05. Space Transition Simplification Mockup

## 목적

현재 Space Transition의 객체 책임 방향은 유지하되, 구현에 남아 있는 불필요한 접착 로직과 우회 흐름을 줄인다.

목표는 다음 네 가지다.

- 코드 양을 줄인다.
- 실행 흐름이 위에서 아래로 바로 읽히게 한다.
- 같은 의미를 여러 함수/상태가 중복해서 표현하지 않게 한다.
- Hub/Battle 같은 특정 공간에 종속되지 않고 기존 Space 간 전환 구조를 그대로 재사용한다.

이번 작업은 새 구조를 추가하는 리팩터가 아니다. 현재 객체 수와 책임 경계를 가능한 한 유지하면서 덜어내는 작업이다.

---

## 유지할 책임 경계

### `AMAMagicCircle`

- Circle 내부 Player 감지와 목록 관리
- Circle 기준 Transform 변환
- 자신의 Mesh를 Transition Visibility 대상으로 등록
- Mask, 전환 순서, 사운드를 소유하지 않음

### `UMAStreamingLevelLoader`

- Streaming Level Load / Unload
- Loaded `AMALevelRoot` 반환
- 전환 순서를 알지 않음

### `UMASpaceTransitionMask`

- 한 로컬 화면의 Mask Close / Closed 유지 / Open
- Post Process와 Material 표시 수명
- Transition Visibility 예외 표시 수명
- 시각 애니메이션에 필요한 자신의 상태만 소유

### `UMASpaceTransitionSubsystem`

- 한 번의 Space Transition 순서 조율
- Destination 준비 대기
- Close 완료 대기
- Player 이동
- Open 완료 대기
- Source 제거와 Destination 승격
- 전환음 요청

### `AMAPlayerControllerBase`

- 서버와 owning client 사이의 RPC 전달 경계
- Space Transition 정책이나 별도 상태를 소유하지 않음

---

## 목표 흐름

구현을 읽었을 때 본질적인 순서가 최대한 직접 보여야 한다.

```text
RequestTransition
-> Load Destination
-> All Clients Ready
-> Close
-> All Clients Closed
-> Move Players
-> Open
-> All Clients Opened
-> Unload Source
-> Promote Destination
```

로컬 Mask의 표현 흐름은 다음 하나다.

```text
Close(SourceCenter)
-> Closed 유지
-> Open(DestinationCenter)
```

이 두 흐름을 성립시키기 위해 필요하지 않은 중간 단계, 접착 함수, 중복 상태 변경은 제거 대상이다.

---

## 확인된 정리 지점

### 1. `PendingPlayers` 등록과 RPC 호출 순서

현재 `TryBeginClose()`와 `CommitHandoff()`는 한 Player를 `PendingPlayers`에 추가한 직후 Client RPC를 호출한다.

Listen Server에서는 로컬 Client RPC 실행 중 서버 상태가 다시 변경될 수 있으므로, 아직 등록되지 않은 다른 Player가 남아 있는 상태에서 Transition이 Abort/완료될 수 있다.

따라서 각 단계의 대기 집합은 RPC 전송 전에 완성되어 있어야 한다.

구체적인 구현 방식은 자유롭게 선택하되 다음 조건을 만족해야 한다.

```text
1. 해당 단계에서 기다릴 Player를 모두 확정
2. Pending 상태를 완성
3. 그 뒤 Client 명령 전송
4. 동기 재진입으로 Phase가 바뀌면 남은 전송을 계속하지 않음
```

### 2. `BeginLocalHandoff()`의 의미 불일치

현재 실제 Player Handoff는 서버의 `MovePlayersToDestination()`에서 수행된다.

그 이후 `BeginLocalHandoff()`가 하는 핵심 작업은 Destination 중심에서 Mask를 Open하는 것이다.

따라서 함수/호출 이름과 실제 책임이 어긋나 있는 부분은 실제 의미에 맞게 정리한다.

이름 변경 자체가 목적은 아니며, 최종 코드에서 `Move`와 `Open`의 책임이 혼동되지 않는 것이 기준이다.

### 3. `UMASpaceTransitionMask`의 접착 함수 다이어트

현재 Mask에는 다음과 같은 내부 함수가 있다.

```text
BeginTransition
CreatePresentation
ApplyPresentation
ReleasePresentation
SetVisibleSubjectsEnabled(bool)
FinishTransition
Reset
BeginDestroy
```

모두 현재 기능 안에서 만들어진 함수지만, 역할이 짧은 구현에 비해 흐름이 여러 함수로 분산되어 있다.

특히 다음을 기준으로 덜어낸다.

- 단순 공통화를 위해 Close/Open의 의미가 숨겨지지 않는가
- `Presentation`처럼 넓은 이름 때문에 실제 작업이 보이지 않는가
- `bool` 하나로 서로 다른 두 작업을 묶고 있지 않은가
- Reset/Destroy/완료 처리에 같은 정리 동작이 흩어져 있지 않은가

필요한 함수는 유지한다. 단순히 함수 수를 줄이기 위해 긴 함수를 만들지는 않는다. 최종 기준은 Close/Open 흐름을 읽기 쉬운가이다.

### 4. 상태의 중복 여부

현재 두 클래스의 `EPhase`는 서로 다른 상태다.

```text
UMASpaceTransitionMask::EPhase
= Open / Closing / Closed / Opening
= 시각 Mask 자체 상태

UMASpaceTransitionSubsystem::EPhase
= Idle / Loading / Closing / Opening
= 전체 전환 절차 상태
```

둘을 하나로 합치는 것을 목표로 하지 않는다.

각 상태는 자신의 책임 안에서 실제 검증과 진행 제어에 필요할 때만 유지한다. 상태 때문에 예외 분기나 중복 검증이 늘어나는 부분이 있다면 상태 자체보다 그 사용 흐름을 먼저 단순화한다.

---

## 제거하지 않을 것

다음은 현재 구조의 확정된 요구이므로 코드 양을 줄인다는 이유로 제거하지 않는다.

- Destination instance identity
- `GenerationSeed`
- Destination이 준비된 뒤에만 Close 시작
- 모든 Player의 Close 완료 뒤 Player 이동
- Source/Destination Circle 상대 Transform 유지
- 같은 Mask 하나로 Source Close -> Destination Open
- 모든 Open 완료 뒤 Source Unload
- Dedicated Server에서 시각 Mask 미생성
- Client RPC 경계로 `AMAPlayerControllerBase` 사용

---

## 추가하지 않을 것

불필요 로직을 줄이는 과정에서 다음을 새로 만들지 않는다.

- 새 Manager
- 새 Coordinator
- 새 Context 객체
- 새 Registry
- 새 Interface
- 새 DataAsset
- 단계별 범용 Framework

현재 객체들만으로 더 단순하게 표현할 수 없다면 그때만 별도 구조를 다시 검토한다.

---

## 완료 기준

- Space Transition의 정상 흐름이 `Load -> Close -> Move -> Open -> Unload`로 바로 읽힌다.
- Listen Server의 동기 재진입으로 `PendingPlayers`나 Phase가 다시 오염되지 않는다.
- Mask는 `Close(SourceCenter) -> Closed -> Open(DestinationCenter)` 외의 전환 의미를 알지 않는다.
- Subsystem은 시각 구현 세부사항을 직접 다루지 않는다.
- PlayerController는 RPC 전달 외의 Transition 상태/정책을 갖지 않는다.
- Hub/Battle 이름이나 특정 Map에 의존하지 않는다.
- 기존 기능을 유지하면서 현재보다 코드와 접착 함수가 줄어든다.
- 단순화를 위해 새로운 추상화 계층을 추가하지 않는다.
