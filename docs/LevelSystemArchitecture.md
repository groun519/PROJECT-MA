# Level System Architecture

이 문서는 레벨 시스템 재구성의 현재 책임 경계와 단계별 연결을 기록한다. 채팅 기록이나 특정 작업 세션의 기억을 설계 근거로 사용하지 않는다.

## 목업 설계 계약

Lobby Hub와 Seamless Transition의 원본 설계 계약은 목업 브랜치의 다음 문서들이다. 관련 작업 전 모두 읽는다.

- `6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md`
- `6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md`
- `b773415987d77118ae6b7ea8fcbea1db6db6d5ed:docs/wip/lobby-rework/05_CameraArchitectureRefactorMockup.md`

현재 작업 트리에 원본 문서를 복제하지 않는다. 다음 명령으로 고정된 버전을 읽는다.

```text
git show 6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md
git show 6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md
git show b773415987d77118ae6b7ea8fcbea1db6db6d5ed:docs/wip/lobby-rework/05_CameraArchitectureRefactorMockup.md
```

## 목표

- 기존 선택 화면 중심의 Lobby를 플레이 가능한 상위 공간인 Hub로 교체한다.
- Hub와 Battle 공간은 마법진 기반 전환으로 연결한다.
- 기존 섹터 이동 시스템은 새 레벨 시스템의 기반으로 확장하지 않는다.
- 목업 단계에서도 최종 책임 경계를 따르며, 임시 연결은 위치와 제거 조건을 명시한다.

## 현재 완료 범위

- `/Game/_Map/LobbyHubMap`을 Editor와 Game의 기본 Lobby 경로로 사용한다.
- 기존 `/Game/_Map/LobbyMap`은 활성 라우팅에서 분리하고 레거시 에셋과 Cook 대상으로 보존한다.
- Hub 전용 GameMode, PlayerController와 Character를 구성했다.
- Arrival Volume에서 실제 Pawn이 생성되고 이동, 회전, 카메라와 시스템 메뉴가 동작한다.
- 전투 어빌리티, 미니맵 캡처, 상태 게이지와 AI Sight 자극원 등록은 Hub에서 비활성화된다.
- 기존 로비 아바타 슬롯, 고정 카메라와 선택 화면용 배치는 `LobbyHubMap`에서 제거했다.
- Loadout과 Party/Invite 기능 오브젝트를 배치하고 기존 Loadout UI 및 Steam Invite 흐름에 연결했다.
- Camera Occlusion Cutout은 일반 Pawn 및 관전 카메라에서 현재 대상을 계속 추적하며, UI Presentation View에서는 프리뷰 대상으로 전환한다.
- UI Presentation View의 로컬 카메라 전환과 Fill Light를 Station 큐브에서 검증했다.
- Magic Circle은 서버에서 영역 진입과 이탈을 판정하고 Ready PlayerState 목록을 복제한다.
- 전원 Ready 출발, Countdown과 공간 전환은 아직 연결하지 않았다.
- Arrival Volume이 원형 영역 안의 무작위 XY 지점에서 WorldStatic 바닥을 찾고, Hitbox가 점유하지 않은 포탈 위치를 Hub GameMode 생성 경로에 연결한다.
- Arrival Volume이 설정 범위 안에서 수직 투입 속도를 한 번 선택하고, Hub Character는 해당 속도를 적용한 뒤 Ragdoll 비행과 착지를 Chaos에 맡긴다.
- Hub Character는 비행 시간 이후 자신의 Ragdoll 안정화 또는 최대 시간을 판정하고, 바닥 복구, Get Up과 입력 재활성화를 닫는다.
- 공용 `UMAAnimInstance::RecoverPose()`가 마지막 Ragdoll Pose 저장, Get Up 재생과 실제 Montage 종료 통지를 닫는다.
- Ragdoll 투입, Get Up과 멀티플레이 동기화가 Hub의 기본 생성 흐름으로 연결되어 있다.
- 중앙 Camera Director를 제거하고 카메라 자체의 보간 상태, 공용 단발 기술, 네트워크 경계와 각 기능의 연출 순서를 실제 책임자에게 분리했다.

## 책임

### `ALobbyHubGameMode`

- Hub의 GameState, PlayerState, PlayerController와 Pawn 클래스를 선택한다.
- Player 생성 시 배치된 Arrival Volume에 Spawn Transform을 요청하고 기존 생성 절차를 실행한다.
- Pawn 생성이 끝나면 같은 Arrival Spawn 값으로 Arrival Volume의 `Launch()` 진입점을 호출한다.
- Arrival Volume이 없거나 유일하지 않으면 검증 오류를 남기고 엔진 기본 생성 경로로 복귀한다.
- 향후 Seamless Travel 사용 가능 상태를 유지한다.
- Spawn 위치 계산, Pawn 내부 초기화, 로드아웃 정책이나 BGM 재생을 소유하지 않는다.

### `ALobbyHubPlayerController`

- 서버 `OnPossess`와 소유 클라이언트 `AcknowledgePossession` 시 Hub Character의 의미 있는 초기화 진입점을 호출한다.
- 로컬 저장 로드아웃과 Loadout Station의 선택 요청을 서버의 `AMAPlayerState`로 전달한다.
- Loadout UI, 프리뷰 카메라, 입력 잠금과 저장 수명을 소유하지 않는다.
- 현재 단계에서는 로컬 Hub 진입 시 `Music.Lobby`를 요청하는 단계 경계가 하나 존재한다.

### `ALobbyHubInteractableActor`

- Hub 기능 오브젝트가 공유하는 상호작용 범위, 표시 Mesh와 Highlight 연결만 소유한다.
- 기능 종류를 표현하는 열거형, 중앙 분기 또는 기능별 실행 정책을 소유하지 않는다.

### `ALobbyHubLoadoutStation`

- Loadout 상호작용부터 기존 UI의 생성·연결·종료, Pending 선택, 저장과 프리뷰 입력 잠금까지 닫는다.
- Loadout용 `UCameraComponent`는 프리뷰 기준점과 화면 구도만 소유한다.
- Station은 ViewTarget, Cutout 대상과 Fill Light 수명을 포함한 로컬 Presentation 진입·복귀 순서를 직접 닫고, 공용 단발 기술만 `FMACameraLibrary`로 호출한다.
- 선택의 서버 전달은 owning Client RPC가 가능한 `ALobbyHubPlayerController::SetLoadoutSelection()` 진입점을 사용한다.

### `ALobbyHubInviteStation`

- Invite 상호작용을 받아 `UMAGameInstance`의 플랫폼 Invite UI 진입점을 호출한다.
- 공통 부모는 어느 기능이 실행되는지 알지 못한다.

### `ALobbyHubMagicCircle`

- Ready 영역과 현재 점유 중인 PlayerState 목록을 소유한다.
- 서버에서 진입과 이탈을 판정하고 목록을 복제한다.
- 외부에는 Ready 여부와 인원 값만 제공하며 표시, Countdown과 공간 전환은 아직 소유하지 않는다.
- 기존 이동 섹터용 `UReadyStateComponent`, `LoopReady`와 아바타 슬롯용 Lobby Ready를 사용하지 않는다.

### `ALobbyHubArrivalVolume`

- 하나의 Sphere Transform과 Radius로 바닥 포탈을 선택할 원형 XY 영역을 정의한다.
- 원형 영역 안의 무작위 지점마다 위에서 아래로 Ray를 내려 WorldStatic 바닥을 찾는다.
- 선택한 바닥 위의 Pawn 생성 영역이 Hitbox Object Channel과 겹치면 해당 후보를 버리고 내부에서 다시 선택한다.
- 유효한 포탈의 Spawn Transform과 Ground Location을 하나의 Arrival Spawn 값으로 제공한다.
- 유효한 빈 바닥을 찾지 못하면 생성을 실패시키며, GameMode는 기존 PlayerStart 경로로 복귀한다.
- 최소·최대 Launch Speed 범위에서 한 값을 선택하고 `Launch()`에서 Character의 `BeginArrival()` 진입점에 한 번 전달한다.
- Character를 발사한 뒤 별도의 목록, Tick 또는 Arrival 진행 상태를 보관하지 않는다.
- Character의 캡슐, Mesh 물리, 애니메이션과 입력 복구 세부사항을 직접 다루지 않는다.

### `UMAGameInstance`

- 플랫폼 Invite UI를 여는 프로젝트 공용 진입점을 소유한다.
- 기존 Lobby와 새 Hub는 동일한 진입점을 사용한다.

### `UMACameraOcclusionCutoutComponent`

- `AMAPlayerControllerBase`가 기본 컴포넌트로 소유한다.
- 로컬 카메라와 현재 Reveal Target을 추적하고 공용 머티리얼 파라미터를 갱신한다.
- 컷아웃 반경과 활성 수명을 닫는다.
- 일반 Pawn 및 관전 카메라에서 활성화되며 Presentation View 동안에는 프리뷰 대상으로 전환된다.
- 외부 호출자는 머티리얼 파라미터 이름이나 계산 방식을 알지 않는다.
- 컷아웃 Material Function을 명시적으로 적용한 환경 머티리얼만 영향을 받는다.

### `UMACameraComponent`

- Player의 기본 `UCameraComponent`를 대체하며 기존 `Cam` subobject 이름을 유지한다.
- `TransitionRig(SpringArm, Settings)`가 Arm Length, Pitch, Target Offset과 FOV 보간을 닫는다.
- 보간 중 새 요청은 현재 Rig 값에서 이어지며 보간이 끝나면 자신의 Tick을 끈다.
- PlayerController, Pawn 탐색, ViewTarget, Fade, Cutout과 기능별 연출 정책을 알지 않는다.

### `FMACameraLibrary`

- ViewTarget 전환, Fade 시작·정지와 Presentation Fill Light 생성·삭제라는 공용 단발 기술만 제공한다.
- UObject가 아닌 stateless C++ API이며 Timer, Callback, 현재 대상과 Presentation 수명을 보관하지 않는다.
- Cutout 대상은 ViewTarget과 다를 수 있으므로 자동으로 선택하지 않는다.
- Shop, Spectate, Hub Loadout과 Legacy Lobby가 각자의 의미와 순서를 소유하고 필요한 기술만 호출한다.

### `AMAPlayerControllerBase`의 카메라 경계

- Possess와 AcknowledgePossession에서 로컬 Cutout 대상을 현재 Pawn으로 연결한다.
- 서버가 owning Client 화면에 요청하는 전체 Fade의 RPC 경계와 그 요청 하나의 Timer만 소유한다.
- ViewTarget, Presentation, Ready Rig와 Legacy Lobby 카메라 시퀀스를 중앙 관리하지 않는다.

### `ALobbyHubCharacter`

- Hub Pawn이 사용하는 런타임 상태만 초기화한다.
- 이동속도와 SlowMultiplier를 설정하고 기존 `State.AbilityBlocked` 정책으로 어빌리티를 차단한다.
- 상속받은 AI Perception과 상태 게이지는 각 기능 컴포넌트 및 위젯의 API로 비활성화한다.
- Arrival 중 자신의 입력, 이동, 캡슐과 Skeletal Mesh Ragdoll 상태를 전환한다.
- 전달받은 수직 초기 속도를 모든 Ragdoll Body에 한 번 적용하며, 이후 루트 위치나 속도를 별도로 유도하지 않는다.
- 비행, 회전, 팔다리 움직임과 착지는 World Gravity와 Chaos 물리가 결정한다.
- 전달받은 수직 속도로 최소 비행 시간을 계산하고, 이후 자신의 Pelvis 속도와 안정 구간 또는 최대 Ragdoll 시간을 기준으로 Get Up을 시작한다.
- Arrival Volume의 캐릭터 목록이나 외부 Tick에 자신의 단계 종료를 의존하지 않는다.
- 현재 Arrival 단계와 해당 단계 진입에 필요한 값을 하나의 서버 권위 상태로 복제한다. Spawn 직후 일회성 Multicast에 초기화를 의존하지 않는다.
- Mesh의 기본 상대 Transform은 Arrival 진입과 무관하게 컴포넌트 초기화에서 보존하며, 클라이언트가 중간 단계를 건너뛰어도 최종 상태가 물리, 부착과 이동을 모두 복구한다.
- 로컬 카메라와 서버 위치 기준이 Ragdoll Pelvis를 따르게 하고, 복구 시 바닥 위치에 캡슐과 Mesh를 다시 결합한다.
- 공용 Player SpringArm의 짧은 위치 Lag이 일반 이동과 복구 시점의 급격한 Pawn 위치 변화를 같은 규칙으로 완화한다.
- 공용 `UMAAnimInstance::RecoverPose()`의 실제 Montage 종료 통지 뒤 이동과 입력을 다시 활성화한다.
- 복구 시작 시 공용 Pose Recovery 진입점에 마지막 물리 자세 보존과 Get Up 출력을 한 번 요청한다.
- 복구 시작 시 마지막 Ragdoll Mesh Transform을 유지하고, Pose 전환과 함께 기본 상대 Transform으로 블렌딩해 루트 위치와 방향의 선행 정렬을 막는다.
- 이 정렬 블렌드는 엎드림/뒤집힘을 구분하지 않는다. 최종 물리 자세 전체를 연속적으로 복구하며, 자세별 Get Up 선택은 필요한 애니메이션이 준비될 때 별도 확장한다.
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

5. 기존 `LobbyMap`은 레거시 에셋과 Cook 대상으로만 보존하며 활성 Lobby 라우팅에 다시 연결하지 않는다.

6. 파티원별 고정 PlayerStart를 새 Hub의 입장 방식으로 사용하지 않는다.
   정상 등장 위치는 Arrival Volume이 소유하며 `LobbyHubMap`에는 PlayerStart를 배치하지 않는다.

7. Camera Occlusion Cutout을 위해 장애물 액터 전체를 숨기거나 충돌 상태를 바꾸지 않는다.
   렌더링 지원 여부는 환경 머티리얼이 명시적으로 선택하고, 런타임 기능은 카메라와 Reveal Target의 의미만 전달한다.

8. Camera Occlusion Cutout과 Presentation View의 Fill Light는 owning Local Player의 화면에만 적용한다.
   서버 상태, 복제 상태와 공간의 기본 조명을 변경하지 않는다.

9. Hub Arrival을 공용 사망, Respawn 또는 `AMACharacter`의 Ragdoll 기능으로 확장하지 않는다.
   현재 투입 연출의 물리 상태와 단계 종료 판단은 `ALobbyHubCharacter` 안에서만 존재한다. `ALobbyHubArrivalVolume`은 위치와 초기 발사 조건만 결정한다.

## 단계 경계 목록

| 현재 위치 | 현재 이유 | 최종 책임자 | 제거 조건 |
|---|---|---|---|
| `ALobbyHubPlayerController::BeginPlay()`의 `Music.Lobby` 요청 | 현재 로컬 Hub 진입을 확정할 활성 공간 객체가 없음 | 향후 Active Space | Active Space가 MusicTag와 활성화 수명을 소유하는 첫 구현에서 Controller 호출 제거 |
| `Hub_LoadoutStation` 큐브의 Cutout 테스트 머티리얼 | 환경 공용 마스터가 확정되기 전에 검증된 렌더링 계약을 유지해야 함 | 환경 공용 마스터 머티리얼 | 승인된 환경 마스터에 Material Function을 연결할 때 테스트 전용 머티리얼 재검토 |

단계 경계를 추가하거나 제거하면 이 표와 해당 코드 주석을 같은 변경에서 갱신한다. 최종 책임자를 아직 정의할 수 없다는 이유만으로 Manager, Registry 또는 범용 Context를 만들지 않는다.

## 다음 순서

1. Magic Circle 중심 Sphere Mask의 Close와 Open 시각 기능을 구현한다.
2. 별도 Battle World와 Seamless Travel을 연결한다.
3. 기존 Battle 섹터와 WaveManager로 FieldReady 이전 전투 연결을 검증한다.
4. 청크, PCG와 Runtime Field 생성은 Seamless Transition 검증 이후 별도 범위에서 연결한다.
5. Battle 공간과 환경 마스터가 확정되면 검증된 Cutout Material Function을 환경 머티리얼에 연결한다.

## 현재 검증 항목

- Arrival Volume에서 Pawn 생성
- 이동, 회전과 Pawn 카메라 추적
- ESC 시스템 메뉴
- 기존 Lobby UI와 전투 UI 미생성
- 어빌리티 차단
- 미니맵, 상태 게이지와 AI Sight 등록 비활성화
- 저장 로드아웃 외형 반영
- Loadout 기능 오브젝트 상호작용, 프리뷰 진입과 저장 종료
- Party/Invite 기능 오브젝트에서 플랫폼 Invite UI 호출
- Loadout 프리뷰 진입 중 캐릭터가 화면 왼쪽에 배치되고 로컬 Fill Light가 정면을 보완
- Presentation View 진입 중 Station 큐브가 캐릭터 주위만 잘라 보이고 종료 시 완전히 복구
- 일반 Pawn 및 관전 카메라에서 Cutout이 현재 대상을 추적하고 Presentation 종료 후 Pawn으로 복구
- Magic Circle 진입과 이탈에 따른 서버 Ready 값과 클라이언트 복제
- 원형 Arrival Area 안의 무작위 WorldStatic 바닥 포탈 선택
- 기존 플레이어 Hitbox 주변을 피한 Spawn Transform 선택
- 수직 초기 속도 이후 완전한 Ragdoll 비행, 실제 바닥 충돌과 팔다리 물리
- 예상 왕복 시간 이후 안정화 판정과 Get Up 및 입력 복구
- Listen Server와 Client에서 투입 표시 및 최종 위치 일치
- 늦게 Pawn을 수신한 Client에서도 Mesh가 캡슐 중심으로 뜨지 않고 현재 Arrival 단계 또는 최종 보행 상태로 복구
- 기존 LobbyMap 회귀 확인
