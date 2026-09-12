# Level System Architecture

이 문서는 레벨 시스템 재구성의 현재 책임 경계와 단계별 연결을 기록한다. 채팅 기록이나 특정 작업 세션의 기억을 설계 근거로 사용하지 않는다.

## 목업 설계 계약

Lobby Hub와 Seamless Transition의 원본 설계 계약은 목업 브랜치의 다음 문서들이다. 관련 작업 전 모두 읽는다.

- `6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md`
- `3ef4ea06eb63585f2ac8e53f1759bbe488ab238f:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md`
- `3ef4ea06eb63585f2ac8e53f1759bbe488ab238f:docs/wip/lobby-rework/03_RuntimeGenerationRebuildMockup.md`
- `6dbca3f2248687ba90977569845a6d893deb1e69:docs/wip/lobby-rework/04_CurrentImplementationFeedbackMockup.md`
- `b773415987d77118ae6b7ea8fcbea1db6db6d5ed:docs/wip/lobby-rework/05_CameraArchitectureRefactorMockup.md`
- `34d6119ee7bf3b3c9683f46adf42cf7cbb8b044a:docs/wip/lobby-rework/06_StreamingLevelFoundationRefactorMockup.md`
- `5f9781a2bf9a3a261ccf46dce0b28c4ff526457d:docs/wip/lobby-rework/07_StencilOwnershipRefactorMockup.md`
- `5909db2a12b40112833a44de4d825fb187a904c7:docs/wip/lobby-rework/04_SpaceTransitionMaskOwnershipRefactorMockup.md`
- `25720efdc68b08a745051edd0e6161deb4625529:docs/wip/lobby-rework/05_SpaceTransitionSimplificationMockup.md`
- `be041a970c5f1ad6de70592b0dab11f44165260e:docs/wip/lobby-rework/06_SpaceDirectionalLightTransitionMockup.md`
- `519792202c53e500a1afbbcab4c7a1da5fd9117f:docs/wip/lobby-rework/07_MagicCircleAutomaticDepartureMockup.md`

현재 작업 트리에 원본 문서를 복제하지 않는다. 다음 명령으로 고정된 버전을 읽는다.

```text
git show 6bbef02cef6e4aa46fc72fef262bfdac43f24684:docs/wip/lobby-rework/01_LobbyHubMockup.md
git show 3ef4ea06eb63585f2ac8e53f1759bbe488ab238f:docs/wip/lobby-rework/02_SeamlessTransitionMockup.md
git show 3ef4ea06eb63585f2ac8e53f1759bbe488ab238f:docs/wip/lobby-rework/03_RuntimeGenerationRebuildMockup.md
git show 6dbca3f2248687ba90977569845a6d893deb1e69:docs/wip/lobby-rework/04_CurrentImplementationFeedbackMockup.md
git show b773415987d77118ae6b7ea8fcbea1db6db6d5ed:docs/wip/lobby-rework/05_CameraArchitectureRefactorMockup.md
git show 34d6119ee7bf3b3c9683f46adf42cf7cbb8b044a:docs/wip/lobby-rework/06_StreamingLevelFoundationRefactorMockup.md
git show 5f9781a2bf9a3a261ccf46dce0b28c4ff526457d:docs/wip/lobby-rework/07_StencilOwnershipRefactorMockup.md
git show 5909db2a12b40112833a44de4d825fb187a904c7:docs/wip/lobby-rework/04_SpaceTransitionMaskOwnershipRefactorMockup.md
git show 25720efdc68b08a745051edd0e6161deb4625529:docs/wip/lobby-rework/05_SpaceTransitionSimplificationMockup.md
git show be041a970c5f1ad6de70592b0dab11f44165260e:docs/wip/lobby-rework/06_SpaceDirectionalLightTransitionMockup.md
git show 519792202c53e500a1afbbcab4c7a1da5fd9117f:docs/wip/lobby-rework/07_MagicCircleAutomaticDepartureMockup.md
```

마지막 04 문서의 Mask 소유권 계약은 앞선 시각 전환 목업의 해당 내용을 대체하며, 05 문서는 그 구조의 실행 흐름을 단순화한다.
현재 구현에서는 06 문서 이후 확정한 책임 절개에 따라 공용 Close/Open 시간축을 `UMASpaceTransitionSubsystem`이 소유하고, Mask는 전달받은 진행도만 표현한다.

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
- Magic Circle의 출발 활성화, 전원 진입 3초 유지·이탈 취소와 서버 출발 알림을 구현하고 사용자 검수를 마쳤다.
- Hub 레벨 BP가 출발 알림을 기존 `RequestTransition(MainMap1)`에 연결한다. 게임 상태에 따른 활성화와 출발 정책은 아직 연결하지 않았다.
- Arrival Volume이 원형 영역 안의 무작위 XY 지점에서 WorldStatic 바닥을 찾고, Hitbox가 점유하지 않은 포탈 위치를 Hub GameMode 생성 경로에 연결한다.
- Arrival Volume이 설정 범위 안에서 수직 투입 속도를 한 번 선택하고, Hub Character는 해당 속도를 적용한 뒤 Ragdoll 비행과 착지를 Chaos에 맡긴다.
- Hub Character는 비행 시간 이후 자신의 Ragdoll 안정화 또는 최대 시간을 판정하고, 바닥 복구, Get Up과 입력 재활성화를 닫는다.
- 공용 `UMAAnimInstance::RecoverPose()`가 마지막 Ragdoll Pose 저장, Get Up 재생과 실제 Montage 종료 통지를 닫는다.
- Ragdoll 투입, Get Up과 멀티플레이 동기화가 Hub의 기본 생성 흐름으로 연결되어 있다.
- Subsystem이 소유한 단일 Mask가 출발 Magic Circle 중심에서 닫히고 목적지 Magic Circle 중심에서 열리는 균열 Sphere Mask 연출을 구현했다.
- Closed에서는 Space 반경을 0까지 닫고, `UMASpaceTransitionVisibilityComponent`가 명시한 Player Mesh와 Magic Circle Mesh만 Custom Stencil 예외로 표시한다.
- `/Game/_Map/WorldRoot`를 Persistent UWorld로 두고 `/Game/_Map/LobbyHubMap`을 초기 Current Space로 블로킹 스트리밍한다.
- `LobbyHubMap`과 `/Game/_Map/MainMap1`은 독립 `.umap`으로 유지하며 각각 정확히 하나의 `AMALevelRoot`와 공용 `AMAMagicCircle`을 가진다.
- `UMAStreamingLevelLoader`가 Destination map을 같은 UWorld의 별도 Slot에 비동기 스트리밍하고 정확히 하나의 `AMALevelRoot`를 결과로 반환한다.
- `UMASpaceTransitionSubsystem`는 서버와 각 클라이언트가 Loader의 Destination 결과를 받은 뒤에만 화면 전환을 시작한다.
- Player는 Source/Destination Magic Circle 기준 상대 Transform을 유지한 채 같은 UWorld 안에서 이동한다.
- Destination Open 완료 뒤 Source streaming instance를 제거하고 Destination을 Current Space로 승격한다.
- 정상 공간 전환 경로는 `ServerTravel`, `TransitionMap`, World 교체와 PlayerStart 도착 복구를 사용하지 않는다.
- 현재 단계는 Hub에서 Battle 테스트 Space로의 Persistent Space Transition을 검증하며, Ready 출발 조건과 Battle 생성은 아직 연결하지 않았다.
- Closed 도중 새로 생성된 대상을 자동 포함하는 기능은 실제 필요가 확인될 때까지 미룬다.
- 전환 기능만 검증하는 `/Game/_Map/MainMap1`에 공용 `AMAMagicCircle`을 구성하고, 기존 `MainMap`은 전환 목업 이전 상태로 보존한다.
- `UMASpaceTransitionSubsystem`는 Destination instance identity와 현재 단계로 요청을 구분하고, 각 단계의 미응답 Player만 모은다.
- 현재 Space Transition은 각 플레이어의 기존 Pawn Camera ViewTarget을 유지하며 Mask만 닫고 연다.
- 각 로컬 화면은 Close 시작부터 Open 완료까지 기존 Pawn Camera의 위치 Lag을 일시 정지해 공간 인계 직후의 보간 이격을 남기지 않는다.
- 중앙 Camera Director를 제거하고 카메라 자체의 보간 상태, 공용 단발 기술, 네트워크 경계와 각 기능의 연출 순서를 실제 책임자에게 분리했다.
- 각 Space의 `AMASpaceDirectionalLight`만 런타임 조명에 참여하며, Open 진행도에 맞춰 Source 조명을 Destination authored state로 전환한 뒤 활성 주체를 넘긴다.
- 공간의 Point/Spot/Rect 조명은 Close 중 원래 광량에서 0으로, Destination Open 중 0에서 원래 광량으로 변화한다. NPC에 부착된 LightComponent도 같은 Level의 수집 대상이다.

## 책임

### `ALobbyHubGameMode`

- Hub의 GameState, PlayerState, PlayerController와 Pawn 클래스를 선택한다.
- Player 생성 시 배치된 Arrival Volume에 Spawn Transform을 요청하고 기존 생성 절차를 실행한다.
- Pawn 생성이 끝나면 같은 Arrival Spawn 값으로 Arrival Volume의 `Launch()` 진입점을 호출한다.
- Arrival Volume이 없거나 유일하지 않으면 검증 오류를 남기고 엔진 기본 생성 경로로 복귀한다.
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

### `AMAMagicCircle`

- Hub와 Battle Space가 공유하는 양방향 마법진이다. 공간 종류를 구분하는 하위 타입을 두지 않는다.
- Ready 영역과 현재 점유 중인 PlayerState 목록을 소유한다.
- 서버에서 진입과 이탈을 판정하고 목록을 복제한다.
- Ready 여부와 인원 값, `SetAutoTravelEnabled()` 및 서버의 `OnAllPlayersReady` 알림을 제공한다. 활성화 중 전원이 3초 머물면 알림을 발행하고 이탈·비활성화 시 대기를 취소한다. 인원/Countdown UI는 없다.
- 자신의 표시 Mesh를 `UMASpaceTransitionVisibilityComponent`에 명시적으로 등록한다.
- `WorldToCircleTransform()`과 `CircleToWorldTransform()`으로 Scale 영향을 제거한 마법진 기준 상대 Transform 변환을 닫는다.
- 목적지 Map, 전환 순서, Mask, 전환음과 전투 상태를 알지 않는다.
- 기존 이동 섹터용 `UReadyStateComponent`, `LoopReady`와 아바타 슬롯용 Lobby Ready를 사용하지 않는다.

### `AMALevelRoot`

- streamed `.umap`의 runtime Level 하나를 대표하며 각 Level map에 정확히 하나만 배치한다.
- 자신의 `TransitionCircle`과 `DirectionalLight` 참조만 제공한다.
- 수명, Bounds, Identity, streaming 구현과 전환 순서를 소유하지 않는다.
- 향후 Hub/Battle별 local reference가 실제로 필요해질 때만 해당 LevelRoot 타입에서 확장한다.
- Battle procedural generation 순서와 결과물은 소유하지 않는다.

### `UMAStreamingLevelLoader`

- `AMALevelRoot`와 함께 `Level/Streaming` 경로에 위치한다.
- `UMASpaceTransitionSubsystem` 내부의 단일 UObject이며 별도 Subsystem, Manager 또는 Factory가 아니다.
- `LoadLevel(LevelMap, InstanceTransform, InstanceIdentity)`이 streaming instance를 만들고, 레벨이 표시되면 그 안의 유일한 `AMALevelRoot`를 결과로 반환한다.
- `UnloadLevel(AMALevelRoot)`이 내부 대응표로 해당 streaming instance를 찾아 제거한다.
- `RegisterInitialLevel()`은 World 시작 시 이미 표시된 streaming level을 한 번 조사해 초기 `AMALevelRoot`와 streaming instance의 대응만 채운다.
- 로드 중 취소, `AMALevelRoot` 탐색과 streaming API 세부사항을 닫는다.
- `OnPreparing`은 레벨 데이터 로드 직후, 컴포넌트 등록과 표시 전에 해당 `ULevel`을 전달한다. 준비 중 조명 차단은 이 경계를 소비하며 Loader는 조명 구현을 알지 않는다.
- Current/Destination 의미, Player 이동, 화면 전환과 Ready 정책을 알지 않는다.
- `GetSwapTransform(CurrentLevel)`은 기존 streaming instance의 배치 Transform에서 반대 슬롯을 계산한다. 초기 슬롯 0과 대기 슬롯 X=100000의 교대 수치는 이 함수 내부에만 두며 별도 토글/슬롯 상태를 저장하지 않는다. 맵 원본과 마법진의 authored 위치는 바꾸지 않는다.

### `UMASpaceTransitionMask`

- `UMASpaceTransitionSubsystem`이 소유하는 단일 `UObject`이며 별도 Actor, Manager 또는 DataAsset을 두지 않는다.
- 호출자에게 중심 좌표를 받는 `Close()`, `Open()`, 정규화된 반경을 적용하는 `SetProgress()`와 종료용 `Reset()`만 제공한다.
- Tick, Phase, Duration과 완료 콜백을 소유하지 않으며 Space 전환 순서와 Source/Destination 의미를 알지 않는다.
- `UMAGameSettings::SpaceTransitionMaterial`로 균열 Sphere Mask의 반경 보간과 현재 World의 Post Process 표시 수명을 닫는다.
- `Close()`에서 현재 World의 `UMASpaceTransitionVisibilityComponent`를 한 번 수집하고 전환 중에만 약한 참조 작업 목록으로 유지한다.
- `Reset()`에서 같은 작업 목록의 대상을 복구하고 목록을 제거한다.
- Dedicated Server에서는 표시 객체를 생성하지 않는다.
- World 교체 감시, Space streaming, 늦게 생성된 대상과 어느 공간으로 이동할지는 소유하지 않는다.

### `AMASpaceDirectionalLight`

- 하나의 Space가 에디터에서 직접 편집하는 실제 Directional Light이자 해당 Space의 authored lighting state를 소유한다.
- Game World에 등록되기 전 자신의 런타임 기여를 끄므로 Destination Space가 미리 표시되어도 한 프레임도 조명에 개입하지 않는다.
- `TransitionTo()` 안에서 회전, 유효 색상과 핵심 광량을 Source authored state에서 Destination authored state로 보간한다.
- 전환 중에는 Source Actor 하나만 조명에 참여하며 Destination authored state는 변경하지 않는다.
- Open 완료 시 Source를 끄고 Destination을 활성화해 다음 전환의 현재 조명 주체를 넘긴다.
- 별도 Tick, Timer, Settings Data와 외부 속성별 보간을 두지 않는다.

### `UMASpaceLightCollector`

- 한 runtime Level의 Point/Spot/Rect `ULocalLightComponent`를 수집하고 원래 광량과 컴포넌트 Soft 참조만 보관한다. BP Construction 재실행으로 교체된 컴포넌트는 같은 객체 경로로 재해석하며 에셋 로드를 요청하지 않는다.
- `Collect(Level, IntensityScale)`로 수집과 최초 배율 적용을 닫는다. Subsystem이 같은 타입의 Source/Destination 작업 객체를 각각 소유한다.
- `SetIntensityScale()`은 항상 저장된 원래 광량에 배율을 곱한다. 현재 광량에 다시 곱해 누적 감소시키지 않는다.
- 색상, 반경, 온도와 Visibility를 변경하지 않으며 원래 꺼진 조명을 켜지 않는다. Static 조명은 제외한다.
- 별도 Tick, Timer, Phase와 RPC를 소유하지 않는다. 수집 이후 파괴되어 교체 대상도 없는 컴포넌트는 건너뛴다.
- `Reset()`은 작업 목록만 비운다. 정상 완료 때 Source 조명을 다시 켜지 않고, 중단 복구는 Subsystem이 Source 배율 1을 요청한 뒤 목록을 해제한다.
- 이번 범위에서는 몬스터 생성 Level과 수명을 변경하지 않는다. 몬스터/프롭의 Close 중 제거 연출과 이미시브 처리는 후속 범위에서 검토한다.
- 현재 계약은 수집된 조명의 고정 광량 페이드다. 전환 중 신규 조명 생성과 다른 기능의 동시 광량 변경, SkyLight/발광 머티리얼/베이크된 간접광의 전환은 별도 범위다.

### `UMASpaceTransitionSubsystem`

- 현재 UWorld 수명의 Subsystem이며 한 번의 Persistent Space Transition 순서를 닫는다.
- 서버의 `RequestTransition(DestinationMap, GenerationSeed)` 요청으로 Destination instance identity를 발급한다. 배치 Transform은 Loader의 `GetSwapTransform()` 결과를 요청 데이터에 넣어 각 클라이언트에 전달한다. 호출자는 좌표를 지정하지 않고, 클라이언트는 서버가 확정한 배치를 그대로 사용한다.
- 서버와 각 클라이언트에서 `UMAStreamingLevelLoader`에 동일한 map, transform과 instance identity를 전달하고 반환된 `AMALevelRoot`를 Destination으로 연결한다.
- 모든 필수 클라이언트가 같은 identity의 Destination 준비를 완료한 뒤에만 각 로컬 화면의 Close를 요청한다.
- 각 단계의 대기 Player 집합을 RPC 전송 전에 완성하고, 동기 재진입으로 단계가 바뀌면 남은 명령을 전송하지 않는다.
- 모든 화면이 닫히면 Player를 Source/Destination Transition Circle 상대 Transform으로 이동한다.
- 각 로컬 Subsystem의 단일 Mask를 Source Circle 중심에서 닫아 유지하고, Player 인계 뒤 같은 Mask를 Destination Circle 중심에서 연다.
- 공용 Close/Open Phase, Duration과 진행도를 소유하고 로컬 애니메이션 중에만 Tick한다.
- 같은 진행도를 Mask에 전달하고 Open에서는 Source `AMASpaceDirectionalLight`의 전환 진입점에도 전달할 뿐 각 표현 객체의 속성과 적용 규칙은 알지 않는다.
- Destination 조명은 Loader의 표시 전 준비 콜백에서 수집해 0으로 둔다. Source 조명은 로컬 Close 시작에 수집하고, 공용 진행도를 Close에서는 Source, Open에서는 Destination LightCollector에 전달한다.
- Source는 광량 0 상태로 Unload한다. Commit 이전 중단은 Source 광량을 복구하고 Destination은 꺼진 상태로 제거한다. 시각 객체는 Dedicated Server에 생성하지 않는다.
- 각 로컬 Subsystem은 Close/Open 단계에 맞춰 Player Camera에 위치 Lag 중지와 복구만 요청한다.
- Close와 Open이 실제 시작될 때 태그 기반 Gameplay Sound를 해당 Magic Circle 위치에서 한 번 요청한다.
- Open 완료 뒤 Loader에 Source 제거를 요청하고 Destination을 Current Space로 승격한다.
- 단계가 다른 응답과 이전 identity 응답은 무시한다. Commit 이전 실패는 Mask를 Reset하고 Destination을 제거한다.
- `AMAPlayerControllerBase`의 RPC는 서버와 각 로컬 `UMASpaceTransitionSubsystem` 사이의 명령과 진행 통지만 전달하며 전환 정책을 소유하지 않는다.
- Ready 판정, Countdown, Pawn 생성과 어느 전투 상태로 진입할지는 소유하지 않는다.

### `UMASpaceTransitionVisibilityComponent`

- 전환 Mask를 통과할 Primitive와 Custom Stencil 최상위 bit의 on/off 의미만 소유한다.
- 자신을 상시 등록하지 않으며 `Close()` 시 현재 World의 `UMASpaceTransitionMask`에 의해 수집된다.
- 전환 중에는 등록된 Primitive의 bit 7만 켜고, Open 완료 시 bit 7만 제거한다.
- Player는 Body, Weapon과 Mount를 직접 등록한다.
- Highlight는 하위 7bit를 독립적으로 사용하며 두 Component는 서로 참조하거나 비활성화하지 않는다.
- 공용 `FMARenderStencil` 연산이 최종 Stencil 값에 맞춰 WriteMask와 CustomDepth 활성 상태를 함께 적용한다.
- Magic Circle은 자신의 표시 Mesh만 직접 등록한다.
- 이 컴포넌트가 없는 객체는 전환 예외 처리와 관련 비용을 갖지 않는다.
- Closed 도중 새로 생성된 대상을 자동 적용하는 책임은 현재 시각 검증 범위에 포함하지 않는다.

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
- `SetLocationLagEnabled()`가 부착된 SpringArm의 위치 Lag 활성 상태 변경을 닫는다.
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

10. Space Transition의 Closed 상태에서 일반 Space Geometry를 남기기 위한 최소 반경을 사용하지 않는다.
    반경은 0까지 닫고 `UMASpaceTransitionVisibilityComponent`가 명시한 Player와 Magic Circle Mesh만 Stencil 예외로 통과시킨다.

11. Hub/Battle 정상 전환에서 `ServerTravel`, `TransitionMap` 또는 UWorld 교체를 사용하지 않는다.
    독립 `.umap`은 같은 `WorldRoot`의 runtime Space Slot에 스트리밍한다.

12. Transition Subsystem이 streaming level 내부를 검색하거나 `ULevelStreaming`을 직접 소유하지 않는다.
    `UMAStreamingLevelLoader`만 로드 완료 시 해당 level 안의 유일한 `AMALevelRoot`를 찾고 내부 대응표로 unload를 닫는다.

13. 한 번의 Closed 인계에서 Source와 Destination용 Mask를 따로 만들지 않는다.
    Subsystem이 소유한 같은 Mask 하나가 Source 중심에서 닫힌 상태를 유지하고 Destination 중심에서 열린다.

14. Streaming 중 Source와 Destination Directional Light를 동시에 활성화하지 않는다.
    Destination `AMASpaceDirectionalLight`는 Game World 등록 전에 비활성화하고, Open 동안 Source 하나를 Destination authored state로 변화시킨 뒤 완료 시 활성 주체만 넘긴다.

15. Mask와 조명은 별도 시간축을 만들지 않는다.
    `UMASpaceTransitionSubsystem`의 같은 진행도를 소비하며 Mask와 조명 객체는 전달받은 값을 자기 표현에만 적용한다.

16. 주변 조명의 페이드는 공간 소속과 원래 광량을 기준으로 한다.
    Destination은 표시 전 차단하고 Source는 0인 채 제거한다. 중단 시 Source만 복구하며 Level 제거 직전에 조명을 재활성화하지 않는다.

## 단계 경계 목록

이름 변경 반영: `MAMagicCircle`은 `bAutoTravelEnabled`, `OnAllPlayersReady`, `SetAutoTravelEnabled`를 사용한다. `BP_MagicCircle`, `LobbyHubMap`, `MainMap1`을 새 이름으로 재저장하고 이번 이름 변경용 Property/Function Redirect 3개를 제거했다. Redirect 없는 새 프로세스에서 BP 컴파일과 두 맵 로드를 수행해 활성화 값 및 레벨 이벤트 바인딩이 유지됨을 확인했다.

| 현재 위치 | 현재 이유 | 최종 책임자 | 제거 조건 |
|---|---|---|---|
| `ALobbyHubPlayerController::BeginPlay()`의 `Music.Lobby` 요청 | 현재 로컬 Hub 진입을 확정할 활성 공간 객체가 없음 | 향후 Active Space | Active Space가 MusicTag와 활성화 수명을 소유하는 첫 구현에서 Controller 호출 제거 |
| `Hub_LoadoutStation` 큐브의 Cutout 테스트 머티리얼 | 환경 공용 마스터가 확정되기 전에 검증된 렌더링 계약을 유지해야 함 | 환경 공용 마스터 머티리얼 | 승인된 환경 마스터에 Material Function을 연결할 때 테스트 전용 머티리얼 재검토 |
| `UMACheatManager::TravelToTransitionTestMap()` / `TravelToSpace()` | 자동 출발과 별개로 왕복 전환을 독립 점검하는 디버그 진입점 | CheatManager | 테스트용 이동 치트가 불필요해지면 제거 |
| `LobbyHubMap` 레벨 BP의 `OnAllPlayersReady -> RequestTransition(MainMap1)` | 게임 진행 정책 연결 전 고정 참가 세션에서 자동 출발을 확인하는 최소 연결 | 향후 게임 진행 상태 책임자 | 게임 상태가 목적지 선택과 출발 알림 소비를 담당할 때 이 BP 연결을 제거 |
| `DefaultEngine.ini`의 `AMASpace -> AMALevelRoot` Class/Property Redirect | 미커밋 Map이 이전 WIP 클래스명과 속성명을 직렬화하고 있어 에디터 재저장 전 Root 참조 유실을 막아야 함 | Level asset migration | `LobbyHubMap`과 `MainMap1`을 `AMALevelRoot.TransitionCircle`로 재저장하고 참조를 확인하면 제거 |

단계 경계를 추가하거나 제거하면 이 표와 해당 코드 주석을 같은 변경에서 갱신한다. 최종 책임자를 아직 정의할 수 없다는 이유만으로 Manager, Registry 또는 범용 Context를 만들지 않는다.

## 다음 순서

1. Standalone과 Listen Server/Client에서 Loader의 initial adopt, Destination 결과, Close, Circle Handoff, Open과 Source unload를 검증한다.
2. Hub 레벨 BP의 자동 전환 연결을 고정 참가 세션에서 검수한다. 이후 접속 경계와 게임 상태의 활성화·목적지 선택 정책을 연결한다.
3. 실제 Battle Space와 LevelManager의 생성/BattleReady를 Destination 준비 조건에 연결한다.
4. 청크, PCG와 Runtime Field 재생성은 LevelManager와 각 Generation Feature Owner의 진입점으로 연결한다.
5. Battle 공간과 환경 마스터가 확정되면 검증된 Cutout Material Function을 환경 머티리얼에 연결한다.

## 07 자동 출발 목업 보완 계약 — 조건 알림 검수 완료, 전환 연결 검수 대기

이 절은 원본 07 목업 이후의 합의다. 조건 알림을 먼저 구현·검수했고, 이어서 Hub 레벨 BP에서 기존 전환 요청을 호출하도록 연결했다. 게임 상태와 신규 접속 정책은 이번 연결에서 구현하지 않는다.

### 이번 구현

- `AMAMagicCircle`에 `SetAutoTravelEnabled(bool)`와 서버의 `OnAllPlayersReady` 알림을 둔다. 활성화 정책은 호출자가 결정하며 출발지/도착지 구분이나 도착 시 자동 비활성화는 넣지 않는다.
- 기존 `PlayersInCircle`을 재사용한다. 활성화 시 현재 조건을 즉시 평가하고, 현재 참가 PlayerState 각각이 모두 안에 있을 때 3초 단발 타이머를 시작한다. 인원수만 비교하지 않는다.
- 한 명이라도 이탈하거나 비활성화되면 타이머를 취소한다. 다시 전원이 모이면 처음부터 3초를 센다. 타이머 진행 중 조건 재평가로 시간을 계속 초기화하지 않는다.
- 완료 시 활성화 및 현재 전원 진입 조건을 다시 확인하고 알림을 한 번 발행한다. 계속 서 있는 동안 주기적으로 재발행하지 않는다. 출발 확정 상태, 자동 비활성화 및 실제 이동 정책은 추가하지 않는다.
- 추가 상태는 활성화 값과 타이머 핸들을 기본으로 한다. 별도 Tick, Countdown UI, 관전 등 아직 없는 기능의 정책 가드는 추가하지 않는다. 서버 권한과 취소·수명 정리는 실제 기능 조건으로 처리한다.
- 조건 알림은 사용자 PIE 검수를 마쳤다. 실제 전환은 참가 명단이 고정된 세션에서 다시 확인한다. 별도 출발 확인 로그는 제거하고 기존 전환 로그를 사용한다.
- `RequestTransition()`은 BP에서 호출 가능하며, `LobbyHubMap` 레벨 BP가 배치된 Circle의 `OnAllPlayersReady`를 받아 `MainMap1`을 요청한다. 좌표 교대는 Loader 내부에서 계산하고, 마법진에는 목적지나 배치 정책을 넣지 않는다. MainMap1의 자동 복귀 바인딩이나 도착 시 활성화 변경은 추가하지 않는다.
- 테스트 시 마법진 인스턴스의 `Auto Travel > Auto Travel Enabled`를 PIE 시작 전에 켠다(기본값 false). 실행 중 변경은 서버에서 `SetAutoTravelEnabled()`를 호출한다. 게임 상태의 활성화 정책, 입력 제한과 합류 시스템은 그대로 후속 범위다.

### 정식 게임 진행 연결 전에 닫을 접속 경계

- 3초 대기 중 참가 명단이 바뀌면 기존 대기를 취소하고 새 명단으로 재평가해야 한다. 접속·퇴장 책임자가 변경을 전달하고, 마법진은 자신의 대기만 갱신한다. 완료 시 재검사만으로 이 경계를 해결했다고 간주하지 않는다.
- 전환 출발과 신규 접속이 동시에 발생하는 경우의 입장 확정은 서버 접속/게임 진행 정책이 담당한다. 마법진에 접속 거절이나 대기열 기능을 넣지 않는다.
- 현재 세션은 진행 중 접속을 허용하고, 전환은 시작 시 준비 요청 대상과 후속 단계에서 순회하는 Controller 목록이 달라질 수 있다. 이번 BP 연결은 고정 참가 세션 검증용이며, 정식 게임 진행 연결 전 참가 확정과 신규 입장 정책을 함께 검수한다. 이번 자동 출발로 중도 접속까지 지원했다고 주장하지 않는다.

### 중도 합류 방향 — 후속 목업

- 진행 중인 판에 신규 사용자가 즉시 참여할 필요는 없다. 신규 사용자는 친구 서버에 미리 접속하지 않고 자신의 로컬 허브에서 준비하며 기다리는 방향으로 잡는다.
- 친구 파티가 허브로 복귀하여 합류 가능한 때 실제 서버에 접속한다. 합류 가능 안내 방식과 직접/자동 합류 여부는 후속 설계에서 확정한다.
- 이를 위해 친구 서버에서 허브와 전투 공간을 상주시키거나 참가자별로 서로 다른 Space를 관리하는 구조로 확장하지 않는다. 현재 파티 전체 이동 모델을 유지한다.
- 접속 가능 상태 전달, 초대 대기, 재접속, UI 및 서버의 입장 확정 구현은 이번 범위가 아니다. 게임 진행/접속 책임자가 나중에 마법진 활성화와 출발 알림 소비를 연결한다.

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
- `TravelToTransitionTestMap` 실행 시 출발 Magic Circle 중심에서 World가 완전히 닫히고 Player와 Magic Circle만 남은 뒤 목적지 중심에서 열리는지 확인
- Host에서 `TravelToSpace /Game/_Map/LobbyHubMap.LobbyHubMap`으로 허브 복귀를 확인한다. 임의 목적지도 `TravelToSpace <맵 오브젝트 경로>`로 요청하며 목적지는 기존 LevelRoot/MagicCircle/DirectionalLight 계약을 충족해야 한다. `TravelToTransitionTestMap`을 포함한 치트의 좌표 인자는 제거했다. 반복 왕복 및 같은 맵 재요청도 Loader가 반대 슬롯을 계산하는지 확인한다.
- Close/Open이 실제 시작될 때 출발지와 목적지에서 태그 기반 전환음이 각각 한 번 재생되는지 확인
- `WorldRoot` 실행 시 초기 `LobbyHubMap`이 Player 생성 전에 로드되고 Loader가 유일한 `AMALevelRoot`를 Current Space로 채택하는지 확인
- Host에서 `TravelToTransitionTestMap` 실행 시 `MainMap1`이 별도 Slot에 준비되고, 준비 중 Hub 플레이가 유지되는지 확인
- Host와 Client가 출발 마법진에서 이루던 상대 위치와 방향을 목적지 Battle 마법진에서도 유지하며, PlayerStart 또는 Pawn 재생성을 사용하지 않는지 확인
- Open 완료 뒤 `LobbyHubMap` streaming instance가 Unload되고 Persistent UWorld와 Controller/Pawn 수명이 유지되는지 확인
- Mask Close/Open 동안 각 플레이어의 기존 Pawn Camera ViewTarget이 바뀌지 않는지 확인
- Close부터 Open 완료까지 Camera Lag이 중지되어 목적지 인계 시 카메라 이격이 없고, 완료·중단 뒤 다시 활성화되는지 확인
- Destination Space가 Close 이전에 로드되어도 Source 화면의 밝기와 색이 변하지 않는지 확인
- Mask Open과 함께 Source 조명이 Destination 조명으로 자연스럽게 변하고 완료 순간 별도 밝기 튐 없이 Destination이 활성 주체가 되는지 확인
- 배치된 Point/Spot/Rect 및 NPC 부착 조명이 Close에서 함께 사라지고 Destination Open에서 원래 밝기로 복구되는지 확인
- 반복 전환 시 광량이 누적 감소하지 않고 원래 꺼진 조명의 Visibility가 유지되는지 확인
- Destination 조명이 표시 전부터 0이고, Source Unload 순간 다시 켜지는 프레임이 없는지 확인
- Close 중단 시 Source 광량이 원래대로 복구되고, 파괴된 조명 참조가 남아도 정리되는지 확인
- 원형 Arrival Area 안의 무작위 WorldStatic 바닥 포탈 선택
- 기존 플레이어 Hitbox 주변을 피한 Spawn Transform 선택
- 수직 초기 속도 이후 완전한 Ragdoll 비행, 실제 바닥 충돌과 팔다리 물리
- 예상 왕복 시간 이후 안정화 판정과 Get Up 및 입력 복구
- Listen Server와 Client에서 투입 표시 및 최종 위치 일치
- 늦게 Pawn을 수신한 Client에서도 Mesh가 캡슐 중심으로 뜨지 않고 현재 Arrival 단계 또는 최종 보행 상태로 복구
- 기존 LobbyMap 회귀 확인
