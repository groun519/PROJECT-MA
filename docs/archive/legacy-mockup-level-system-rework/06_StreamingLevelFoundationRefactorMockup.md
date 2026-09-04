# 06. Streaming Level Foundation Refactor Mockup

> Status: Final implementation contract / implementation review passed
> Target branch: feature/psw/level-system-rework
> Reviewed local-change base HEAD: 55665930303a99f3fdaf3d8c29a0422aee53c324
> Mockup branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8
> Supersedes: 04_CurrentImplementationFeedbackMockup.md의 AMASpace / UMASpaceLoader / Initial Space naming contract

## 1. 목적

현재 WIP의 AMASpace와 UMASpaceLoader는 기능적으로는 가벼워졌지만, Space라는 이름이 두 가지 의미로 사용된다.

현재 코드를 처음 읽으면 다음처럼 보인다.

~~~text
UMASpaceLoader
-> LoadSpace()
-> AMASpace
~~~

하지만 실제 구현은 다음이다.

~~~text
.umap
-> Streaming Level load
-> loaded ULevel
-> 그 Level 안의 대표 Actor 탐색
-> 대표 Actor가 TransitionCircle 제공
~~~

즉 실제로 로드/언로드되는 것은 Streaming Level이고, 대표 Actor는 그 Level의 외부 진입점을 제공하는 Root다.

이번 수정의 목적은 이 두 개념을 이름에서 명확하게 분리하고, Persistent World + Streaming Level 기반을 Transition 세부 로직과 분리된 첫 선커밋 단위로 닫는 것이다.

핵심 원칙:

> Level은 실제로 streaming되는 .umap runtime instance를 의미한다.
>
> LevelRoot는 해당 streamed Level을 대표하는 Actor다.
>
> Loader는 Level streaming 구현만 닫는다.
>
> Current/Destination, Player handoff, Close/Open 순서는 TransitionSubsystem의 책임으로 남긴다.
>
> 아직 검수하지 않은 Transition 로직은 이번 선커밋에 섞지 않는다.

## 2. 최종 큰 구조

~~~text
WorldRoot
|
+-- initial streamed Level: LobbyHubMap
|   +-- AMALevelRoot
|       +-- TransitionCircle
|
+-- runtime streamed Level: MainMap1
    +-- AMALevelRoot
        +-- TransitionCircle

UMAStreamingLevelLoader
|
+-- RegisterInitialLevel()
+-- LoadLevel()
+-- UnloadLevel()
+-- CancelPendingLoad()
+-- FindLevelRoot()
~~~

책임은 다음처럼 고정한다.

~~~text
AMALevelRoot
= streamed Level 하나를 대표하고 현재는 TransitionCircle 참조만 제공한다.

UMAStreamingLevelLoader
= Level map을 같은 UWorld에 streaming하고, 그 Level의 유일한 AMALevelRoot를 반환하며, 해당 Level을 unload한다.

UMASpaceTransitionSubsystem
= 이번 목업의 구현 대상이 아니다.
= 이후 LevelRoot와 StreamingLevelLoader를 소비하며 transition 순서만 소유한다.
~~~

## 3. AMASpace -> AMALevelRoot

파일 위치도 개념 변경과 함께 옮긴다.

~~~text
Level/Space/MASpace.h
-> Level/Streaming/MALevelRoot.h
~~~

현재 AMASpace는 자체 streaming, lifecycle, generation, identity를 소유하지 않는다.

실제 역할은 다음 하나다.

~~~text
loaded Level의 대표 Actor
-> TransitionCircle
~~~

따라서 이름을 다음처럼 변경한다.

~~~text
AMASpace
-> AMALevelRoot
~~~

현재처럼 별도 runtime 동작이 없다면 header-only를 유지한다.

필수 데이터:

~~~cpp
UPROPERTY(EditInstanceOnly, Category = "Level")
TObjectPtr<AMAMagicCircle> TransitionCircle;
~~~

외부 진입점:

~~~cpp
AMAMagicCircle* GetTransitionCircle() const;
~~~

추가하지 않을 것:

- SceneRoot
- 생성자
- Bounds
- Level identity
- lifecycle enum
- Prepare()
- Activate()
- Retire()
- streaming handle
- generation state
- generic validation API

## 4. UMASpaceLoader -> UMAStreamingLevelLoader

파일 위치도 함께 옮긴다.

~~~text
Level/Space/MASpaceLoader.h/.cpp
-> Level/Streaming/MAStreamingLevelLoader.h/.cpp
~~~

현재 Loader의 실제 핵심 구현은 ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr()이다.

따라서 이름을 다음처럼 변경한다.

~~~text
UMASpaceLoader
-> UMAStreamingLevelLoader
~~~

목표 public API:

~~~cpp
AMALevelRoot* RegisterInitialLevel();

bool LoadLevel(
    TSoftObjectPtr<UWorld> LevelMap,
    const FTransform& InstanceTransform,
    const FString& InstanceIdentity,
    FOnMALevelLoaded OnLoaded);

void UnloadLevel(AMALevelRoot& LevelRoot);
void CancelPendingLoad();
~~~

private API:

~~~cpp
void HandleLevelShown();
AMALevelRoot* FindLevelRoot(ULevelStreaming& StreamingLevel) const;
void ReleaseStreamingLevel(ULevelStreaming& StreamingLevel) const;
~~~

내부 상태:

~~~text
LoadedLevels
PendingStreamingLevel
PendingLoadedDelegate
~~~

delegate 이름도 Level 기준으로 맞춘다.

~~~text
FOnMASpaceLoaded
-> FOnMALevelLoaded
~~~

로그 카테고리도 맞춘다.

~~~text
LogMASpaceLoader
-> LogMAStreamingLevelLoader
~~~

## 5. RegisterInitialLevel()

기존 AdoptInitialSpace()는 이름만 보고 실제 동작을 파악하기 어렵다.

최종 이름:

~~~text
AdoptInitialSpace()
-> RegisterInitialLevel()
~~~

의미:

> Loader 외부에서 World 시작 시 이미 로드되어 있는 초기 Streaming Level을 찾아 Loader의 관리 대상으로 등록한다.

헤더에는 이 의미가 드러나는 간단한 주석을 둔다.

~~~cpp
// Registers the initial streaming level that was loaded outside this loader.
AMALevelRoot* RegisterInitialLevel();
~~~

동작:

~~~text
WorldRoot 시작
-> visible Streaming Level 확인
-> loaded ULevel 확인
-> AMALevelRoot를 가진 Level 탐색
-> 정확히 하나인지 검증
-> LoadedLevels에 LevelRoot <-> ULevelStreaming 등록
-> AMALevelRoot 반환
~~~

### 제거할 guard

현재의 다음 검사는 제거한다.

~~~cpp
ensureMsgf(LoadedSpaces.IsEmpty(), ...)
~~~

이유:

- RegisterInitialLevel()은 bootstrap 시 한 번 호출되는 명확한 진입점이다.
- LoadedLevels.IsEmpty() 여부는 초기 등록 여부 자체를 정확히 표현하는 상태가 아니다.
- 별도 bool/state를 추가해서 이 호출을 막지도 않는다.

### 유지할 검증

다음은 Level asset contract이므로 유지한다.

~~~text
초기 visible LevelRoot가 0개
-> 실패

초기 visible LevelRoot가 2개 이상
-> 실패
~~~

별도 Validator 객체는 만들지 않는다.

## 6. LoadLevel()

기존 LoadSpace()는 LoadLevel()로 변경한다.

이 함수는 Streaming Level을 만드는 함수라는 사실을 이름에서 바로 드러낸다.

목표 흐름:

~~~text
LevelMap null 확인
-> 동시에 다른 Pending Level load가 없는지 ensure
-> LoadLevelInstanceBySoftObjectPtr()
-> PendingStreamingLevel 저장
-> OnLevelShown bind
-> shown 완료
-> FindLevelRoot()
-> LoadedLevels 등록
-> OnLoaded(LevelRoot)
~~~

### 유지할 guard

동시에 두 Level을 load하려는 요청은 현재 Loader의 상태 계약 위반이다.

현재 Loader는 다음 상태를 하나씩만 가진다.

~~~text
PendingStreamingLevel
PendingLoadedDelegate
~~~

따라서 다음 ensure는 유지한다.

~~~text
Only one Level can load at a time.
~~~

LevelMap.IsNull()은 일반 실패이므로 ensure를 추가하지 않고 false를 반환한다.

## 7. FindLevelRoot()

기존 FindSpace()는 FindLevelRoot()로 변경한다.

동작은 제한된 lookup으로 유지한다.

~~~text
specific ULevelStreaming
-> GetLoadedLevel()
-> 그 ULevel의 Actor 목록
-> AMALevelRoot 탐색
~~~

이것은 World global search, Registry, Manager가 아니다.

Level 하나에는 정확히 하나의 AMALevelRoot가 있어야 한다.

유지할 ensure:

~~~text
AMALevelRoot 0개
-> 유효한 Level 결과를 만들 수 없으므로 실패

AMALevelRoot 2개 이상
-> 대표 Root를 결정할 수 없으므로 실패
~~~

메시지도 Space가 아니라 Level/LevelRoot 기준으로 수정한다.

## 8. UnloadLevel() / CancelPendingLoad()

기존 UnloadSpace()는 UnloadLevel()로 변경한다.

외부는 ULevelStreaming을 알 필요가 없다.

~~~text
AMALevelRoot
-> LoadedLevels 대응표
-> ULevelStreaming
-> ReleaseStreamingLevel()
~~~

CancelPendingLoad()는 그대로 유지한다.

아직 AMALevelRoot 결과가 확정되지 않은 pending load를 취소해야 하므로, 이 단계에서는 PendingStreamingLevel을 직접 정리하는 것이 맞다.

ReleaseStreamingLevel()도 그대로 유지한다.

~~~text
SetShouldBeVisible(false)
SetShouldBeLoaded(false)
SetIsRequestingUnloadAndRemoval(true)
~~~

이 엔진 streaming 세부 구현은 Loader 밖으로 노출하지 않는다.

## 9. 파일 위치 계약

이번 rename은 타입 이름만 바꾸고 기존 `Level/Space` 폴더를 남기지 않는다.

최종 위치:

~~~text
Source/P_MA/Private/Level/Streaming/
├─ MALevelRoot.h
├─ MAStreamingLevelLoader.h
└─ MAStreamingLevelLoader.cpp
~~~

기존:

~~~text
Source/P_MA/Private/Level/Space/
├─ MASpace.h
├─ MASpaceLoader.h
└─ MASpaceLoader.cpp
~~~

는 제거한다.

이유:

> streamed Level과 그 대표 Root를 명확하게 표현하기 위해 타입 이름을 바꾸는 것이므로, 폴더 구조도 같은 개념을 표현해야 한다.

`Level/Space`를 남긴 채 파일명만 바꾸지 않는다.

## 10. Map / Asset 의존성 결과와 커밋 분리

Map 의존성 검사는 이미 수행되었고, 첫 Foundation 커밋에서 분리해야 하는 강한 참조가 확인됐다.

~~~text
LobbyHubMap
└─ BP_MagicCircle 강참조

MainMap1
└─ BP_MagicCircle 강참조

WorldRoot
└─ LobbyHubMap 초기 Streaming 참조
~~~

따라서 첫 Foundation WIP에는 다음을 넣지 않는다.

~~~text
WorldRoot.umap
LobbyHubMap.umap
MainMap1.umap
BP_MagicCircle.uasset

DefaultEngine.ini
DefaultGame.ini
LobbyHubGameMode 관련 변경
~~~

맵 구조나 Config 방향이 잘못됐다는 의미가 아니다.

첫 WIP의 목적이 이미 검수 완료된 Streaming Level 기반만 독립적으로 닫는 것이므로, 아직 미커밋 Transition asset을 요구하는 Map/Config/GameMode 묶음은 다음 Transition/Asset 변경 단위로 미룬다.

이렇게 하면 첫 WIP가 BP_MagicCircle 및 아직 미검수인 Transition Component 구현을 의존성 때문에 끌어오지 않는다.

## 11. LevelSystemArchitecture.md 동기화

실제 구현과 같은 변경에서 `docs/LevelSystemArchitecture.md`의 관련 부분도 갱신한다.

반영 범위:

~~~text
AMASpace
-> AMALevelRoot

UMASpaceLoader
-> UMAStreamingLevelLoader

LoadSpace / UnloadSpace / AdoptInitialSpace
-> LoadLevel / UnloadLevel / RegisterInitialLevel

Level/Space
-> Level/Streaming

06_StreamingLevelFoundationRefactorMockup.md 계약 참조 추가
~~~

단, 문서 안의 다른 Transition 설명을 이번 WIP에 끌어오지 않는다.

현재 작업 트리의 `docs/LevelSystemArchitecture.md`에 다른 미검수 Transition 수정이 함께 존재한다면 관련 hunk만 부분 스테이징한다.

목표:

> 코드와 현재 Architecture 문서가 같은 이름과 같은 책임 경계를 가리키게 한다.

## 12. 첫 Foundation WIP의 정확한 범위

첫 WIP의 의미:

> Streaming Level을 load/unload하고 해당 Level의 대표 Root를 얻는 최소 기반을 확정한다.

스테이징 대상:

~~~text
Source/P_MA/Private/Level/Streaming/MALevelRoot.h
Source/P_MA/Private/Level/Streaming/MAStreamingLevelLoader.h
Source/P_MA/Private/Level/Streaming/MAStreamingLevelLoader.cpp

docs/LevelSystemArchitecture.md
- 이번 Foundation 명칭/책임 변경 관련 hunk
- 06 목업 계약 참조 hunk
~~~

첫 WIP에 Map, Config, GameMode, BP_MagicCircle, Transition policy 구현을 넣지 않는다.

나머지 로컬 Transition 코드는 빌드 가능한 작업 트리를 유지하기 위해 새 이름을 참조하도록 함께 수정할 수 있다.

예:

~~~text
AMASpace 참조
-> AMALevelRoot

UMASpaceLoader 참조
-> UMAStreamingLevelLoader
~~~

그러나 그 수정은 첫 WIP에 스테이징하지 않는다.

## 13. 이번 선커밋에 넣지 않을 것

아직 세부 검수가 끝나지 않은 Transition 묶음은 이번 commit에 넣지 않는다.

~~~text
WorldRoot.umap
LobbyHubMap.umap
MainMap1.umap
BP_MagicCircle.uasset
DefaultEngine.ini의 WorldRoot/Redirect 관련 변경
DefaultGame.ini의 MapsToCook 관련 변경
LobbyHubGameMode 관련 변경

MAMagicCircle의 transition 변경
MASpaceTransitionMaskComponent
MASpaceTransitionVisibilityComponent
MASpaceTransitionTypes
MASpaceTransitionSubsystem
MAPlayerControllerBase의 Space Transition RPC
MAWorldTransitionCoordinator 제거
WorldTransition Mask/Visibility 제거
Transition GameplayTag rename
Transition Cheat flow
Player handoff
Close / Handoff / Open
~~~

이 파일들은 LevelRoot / StreamingLevelLoader 새 이름을 참조하도록 로컬 작업 트리에서 기계적으로 수정할 수는 있다.

하지만 그 변경이 첫 선커밋에 들어간다는 의미는 아니다.

## 14. 구현 리뷰 결과

2026-09-03 최신 Local Changes Review 기준으로 Foundation 구현은 본 계약과 일치한다.

확인된 실제 파일:

~~~text
Source/P_MA/Private/Level/Streaming/MALevelRoot.h
Source/P_MA/Private/Level/Streaming/MAStreamingLevelLoader.h
Source/P_MA/Private/Level/Streaming/MAStreamingLevelLoader.cpp
~~~

확인 결과:

~~~text
AMALevelRoot
- TransitionCircle만 보유
- SceneRoot / 생성자 / lifecycle 없음
- header-only 유지

UMAStreamingLevelLoader
- RegisterInitialLevel()
- LoadLevel()
- UnloadLevel()
- CancelPendingLoad()
- FindLevelRoot()
- LoadedLevels
- PendingStreamingLevel
- PendingLoadedDelegate
~~~

검수 판정:

> Foundation 코드 통과.

특히 다음 계약이 구현에 반영됐다.

- `RegisterInitialLevel()`에 LoadedLevels empty 여부로 1회 호출을 강제하는 guard가 없다.
- 초기 visible LevelRoot 0개/복수 검증은 유지한다.
- 동시에 두 Level을 load하려는 요청은 ensure로 드러낸다.
- null LevelMap은 일반 실패로 처리한다.
- `HandleLevelShown()`은 pending 상태를 정리한 뒤 callback을 실행하므로 callback에서 다음 load를 시작할 수 있다.
- streaming API 세부사항은 Loader 내부에 남아 있다.
- 파일 위치가 `Level/Streaming`으로 정리됐다.

첫 WIP 전 남은 문서 정리는 하나다.

~~~text
docs/LevelSystemArchitecture.md

"각 Space map에 정확히 하나만 배치한다."
-> "각 Level map에 정확히 하나만 배치한다."
~~~

이 변경도 Foundation 관련 hunk에 포함한다.

Map / Config / GameMode / BP_MagicCircle / Transition policy 변경은 여전히 첫 WIP 범위 밖이다.

## 15. Acceptance Criteria

### Naming

- 코드에서 streamed Level과 대표 Actor의 의미가 Space 하나로 혼용되지 않는다.
- 대표 Actor 이름은 AMALevelRoot다.
- Loader 이름은 UMAStreamingLevelLoader다.
- public API는 RegisterInitialLevel / LoadLevel / UnloadLevel / CancelPendingLoad로 읽힌다.
- 내부 lookup은 FindLevelRoot로 읽힌다.

### Minimality

- AMALevelRoot는 TransitionCircle만 가진 header-only Actor다.
- 불필요한 SceneRoot/생성자/lifecycle/identity/bounds가 없다.
- Loader는 현재 필요한 pending load 하나와 LevelRoot <-> StreamingLevel 대응만 보관한다.
- Initial registration 전용 Manager/Registry/Provider가 없다.
- RegisterInitialLevel() 1회 호출을 강제하기 위한 별도 상태나 불필요한 ensure가 없다.

### Validation

- LevelRoot 0개는 실패한다.
- LevelRoot 2개 이상은 실패한다.
- 동시에 두 Level을 load하려는 호출은 ensure로 드러난다.
- null LevelMap은 일반 실패로 처리한다.
- pending load 취소가 OnLevelShown delegate와 streaming instance를 정리한다.

### Separation

- Loader 코드에 Current/Destination 의미가 없다.
- Loader 코드에 Player, RPC, MagicCircle transition 순서가 없다.
- TransitionSubsystem이 ULevelStreaming 세부 구현을 직접 새로 소유하지 않는다.
- 이번 선커밋은 아직 미검수 Transition policy를 포함하지 않는다.

### Commit

- 첫 WIP에는 MALevelRoot, MAStreamingLevelLoader, LevelSystemArchitecture 관련 hunk만 들어간다.
- Map / Config / GameMode / BP_MagicCircle은 첫 WIP에서 제외한다.
- 나머지 Transition 코드는 새 타입명을 참조하도록 수정할 수 있지만 unstaged로 남긴다.
- docs/LevelSystemArchitecture.md는 06 계약과 새 명칭을 같은 변경에서 반영한다.
- build와 git diff --check를 통과한다.

## 16. 구현 후 읽혀야 하는 형태

~~~text
WorldRoot starts
-> UMAStreamingLevelLoader::RegisterInitialLevel()
-> AMALevelRoot

later

UMAStreamingLevelLoader::LoadLevel(...)
-> ULevelStreamingDynamic
-> OnLevelShown
-> FindLevelRoot()
-> AMALevelRoot

UMAStreamingLevelLoader::UnloadLevel(LevelRoot)
-> internal ULevelStreaming lookup
-> unload/removal
~~~

처음 프로젝트를 보는 사람은 Space라는 내부 용어를 알아야만 이 코드를 이해해서는 안 된다.

이 첫 선커밋의 목표는 기능 추가가 아니라, 실제 엔진 개념인 Streaming Level과 그 대표 Actor를 이름과 책임에서 정확하게 표현하는 것이다.
