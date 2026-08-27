# Level System Architecture

이 문서는 레벨 시스템 재구성의 현재 책임 경계와 단계별 연결을 기록한다. 채팅 기록이나 특정 작업 세션의 기억을 설계 근거로 사용하지 않는다.

## 목업 설계 계약

Lobby Hub와 Seamless Transition의 원본 설계 계약은 목업 브랜치의 다음 문서다. 관련 작업 전 두 문서를 모두 읽는다.

- `6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md`
- `6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md`

현재 작업 트리에 원본 문서를 복제하지 않는다. 다음 명령으로 고정된 버전을 읽는다.

```text
git show 6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md
git show 6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md
```

## 목표

- 기존 선택 화면 중심의 Lobby를 플레이 가능한 상위 공간인 Hub로 교체한다.
- Hub와 Battle 공간은 마법진 기반 전환으로 연결한다.
- 기존 섹터 이동 시스템은 새 레벨 시스템의 기반으로 확장하지 않는다.
- 목업 단계에서도 최종 책임 경계를 따르며, 임시 연결은 위치와 제거 조건을 명시한다.

## 현재 완료 범위

- 기존 `/Game/_Map/LobbyMap`과 현재 시작 경로는 보존한다.
- 작업용 `/Game/_Map/LobbyHubMap`을 별도 생성했다.
- Hub 전용 GameMode, PlayerController와 Character를 구성했다.
- Hub PlayerStart에서 실제 Pawn이 생성되고 이동, 회전, 카메라와 시스템 메뉴가 동작한다.
- 전투 어빌리티, 미니맵 캡처, 상태 게이지와 AI Sight 자극원 등록은 Hub에서 비활성화된다.
- 기존 로비 아바타 슬롯, 고정 카메라와 선택 화면용 배치는 `LobbyHubMap`에서 제거했다.
- Loadout, Party/Invite, Magic Circle과 Spawn/Arrival 기능 오브젝트는 아직 배치하거나 연결하지 않았다.

## 책임

### `ALobbyHubGameMode`

- Hub의 GameState, PlayerState, PlayerController와 Pawn 클래스를 선택한다.
- 향후 Seamless Travel 사용 가능 상태를 유지한다.
- Pawn 내부 초기화, 로드아웃 정책이나 BGM 재생을 소유하지 않는다.

### `ALobbyHubPlayerController`

- 서버 `OnPossess`와 소유 클라이언트 `AcknowledgePossession` 시 Hub Character의 의미 있는 초기화 진입점을 호출한다.
- 로컬 저장 로드아웃을 서버의 `AMAPlayerState`로 전달한다.
- 현재 단계에서는 로컬 Hub 진입 시 `Music.Lobby`를 요청하는 단계 경계가 하나 존재한다.

### `ALobbyHubCharacter`

- Hub Pawn이 사용하는 런타임 상태만 초기화한다.
- 이동속도와 SlowMultiplier를 설정하고 기존 `State.AbilityBlocked` 정책으로 어빌리티를 차단한다.
- 상속받은 AI Perception과 상태 게이지는 각 기능 컴포넌트 및 위젯의 API로 비활성화한다.
- `AMACharacter::ServerSideInit()`을 호출하지 않는다.

### `UMAMusicSubsystem`

- MusicTag 해석, BGM 재생, 교체, 페이드와 수명을 닫는다.
- 어느 MusicTag를 사용할지 결정하지 않는다. 최종 선택 책임은 활성 공간에 있다.

### `AMAPlayerState`

- 네트워크에서 공유되는 현재 로드아웃 선택을 소유한다.
- 로컬 저장 파일을 직접 읽지 않는다.

## 불변 규칙

1. Hub Character에 전투용 `ServerSideInit()`을 호출하지 않는다.
   현재 플레이어 기본 스탯 조회는 정확한 Actor Class 일치를 사용한다. 별도 Hub 클래스는 전투 스탯 행이 없으며, 전투 초기화는 Hub에 필요하지 않은 GE와 Ability까지 부여한다.

2. Hub의 어빌리티 차단을 별도 입력 플래그로 재구현하지 않는다.
   기존 `State.AbilityBlocked` 정책을 사용한다.

3. 상속 기능을 끄기 위해 `AMACharacter`의 내부 상태를 노출하지 않는다.
   Perception 컴포넌트와 상태 게이지 위젯의 기존 진입점을 사용한다.

4. Hub Controller는 기존 `ALobbyPlayerController`를 상속하지 않는다.
   준비 UI, 아바타 슬롯, 고정 카메라와 기존 로비 전환 책임이 새 Hub로 유입되어서는 안 된다.

5. 기존 `LobbyMap`과 기본 라우팅은 명시적인 전환 범위가 승인되기 전까지 유지한다.

6. 파티원별 고정 PlayerStart를 새 Hub의 입장 방식으로 사용하지 않는다.
   현재 `HubPlayerStart`는 Pawn 동작 검증을 위한 단일 생성 기준점이다. 최종 등장 위치와 투입 과정은 Spawn/Arrival 기능이 소유한다.

## 단계 경계 목록

| 현재 위치 | 현재 이유 | 최종 책임자 | 제거 조건 |
|---|---|---|---|
| `ALobbyHubPlayerController::BeginPlay()`의 `Music.Lobby` 요청 | 현재 로컬 Hub 진입을 확정할 활성 공간 객체가 없음 | 향후 Active Space | Active Space가 MusicTag와 활성화 수명을 소유하는 첫 구현에서 Controller 호출 제거 |
| `LobbyHubMap`의 단일 `HubPlayerStart` | Spawn/Arrival 구현 전 Pawn 생성과 조작을 검증해야 함 | 향후 Spawn/Arrival 기능 | Spawn/Arrival 기능이 최초 생성 위치와 Ragdoll 투입 시작을 소유할 때 현재 배치 재검토 |

단계 경계를 추가하거나 제거하면 이 표와 해당 코드 주석을 같은 변경에서 갱신한다. 최종 책임자를 아직 정의할 수 없다는 이유만으로 Manager, Registry 또는 범용 Context를 만들지 않는다.

## 다음 순서

1. Loadout과 Party/Invite 기능 오브젝트를 배치하고 기존 UI 및 Steam Invite 흐름에 연결한다.
2. 원기둥 Placeholder로 Magic Circle의 위치와 점유 공간을 확정한다.
3. Magic Circle 진입과 이탈을 서버 권한 Ready 상태로 연결한다.
4. Spawn Volume 기반 Ragdoll Arrival과 Get Up을 구현한다.
5. 검증된 Hub를 기본 Lobby 경로로 전환하고 기존 `LobbyMap`은 레거시 자산으로 보존한다.
6. Magic Circle 중심 Sphere Mask와 별도 Battle World 간 Seamless Travel을 구현한다.
7. 기존 Battle 섹터와 WaveManager로 FieldReady 이전 전투 연결을 검증한다.
8. 청크, PCG와 Runtime Field 생성은 Seamless Transition 검증 이후 별도 범위에서 연결한다.

## 현재 검증 항목

- Hub PlayerStart에서 Pawn 생성
- 이동, 회전과 Pawn 카메라 추적
- ESC 시스템 메뉴
- 기존 Lobby UI와 전투 UI 미생성
- 어빌리티 차단
- 미니맵, 상태 게이지와 AI Sight 등록 비활성화
- 저장 로드아웃 외형 반영
- 기존 LobbyMap 회귀 확인
