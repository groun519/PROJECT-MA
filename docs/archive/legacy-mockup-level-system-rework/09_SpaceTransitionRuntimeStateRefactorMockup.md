# 09. Space Transition Runtime State Refactor Mockup

## 목적

현재 `UMASpaceTransitionSubsystem`은 한 번에 하나의 Space Transition만 실행하는 구조다.

기존 구현에는 이 실제 요구보다 많은 상태와 신호가 들어가 있었다.

```text
RequestId
NextRequestId
LastAcceptedRequestId
IsActiveRequest()
EMASpaceTransitionClientMilestone
LocalSourceLevel
LocalDestinationLevel
```

이 리팩터의 목표는 단순히 코드를 짧게 만드는 것이 아니다.

> 현재 진행 중인 Transition 하나만 정확하게 모델링하고,
> 그 Transition에 필요한 데이터만 Transition 수명 동안 유지한다.
> Transition이 끝나면 관련 Runtime 상태는 모두 사라진다.
> 과거 Transition을 위한 카운터, 히스토리, 별도 세대 상태는 만들지 않는다.

이번 최종 정리에서는 이미 적용된 큰 구조 단순화에 더해 다음 군더더기까지 제거한다.

```text
PendingPlayers Contains + Remove 중복
HandleClientProgress의 중복 Idle 방어
PromoteDestination / DiscardDestination 내부의 DestinationLevel.Reset 중복
```

또한 짧은 단일 Guard는 프로젝트의 기존 스타일에 맞춰 한 줄로 유지한다.

```cpp
if (PendingPlayers.Remove(&PlayerController) == 0) return;
```

---

# 1. 최종 구조 원칙

## 1.1 한 번에 하나의 Transition만 존재한다

`UMASpaceTransitionSubsystem`은 `Idle`일 때만 새 Transition을 시작한다.

```text
Idle
 ↓
Loading
 ↓
Closing
 ↓
Opening
 ↓
Idle
```

새 Transition Queue는 만들지 않는다.

```cpp
if (Phase != EPhase::Idle) return false;
```

현재 Transition의 정체는 별도의 `TransitionId`나 `RequestId`가 아니라 현재 상태 자체다.

```text
Phase
ActiveRequest
CurrentLevel
DestinationLevel
PendingPlayers
```

---

## 1.2 Transition 사이에 유지되는 상태와 Transition 전용 상태를 구분한다

World 수명 동안 유지:

```text
LevelLoader
CurrentLevel
```

현재 Transition 동안만 유지:

```text
Phase
ActiveRequest
DestinationLevel
PendingPlayers
```

Transition 완료 또는 Abort 후:

```text
Phase = Idle
ActiveRequest = Empty
DestinationLevel = null
PendingPlayers = Empty
```

`CurrentLevel`만 다음 Transition까지 유지된다.

---

# 2. 최종 Subsystem 상태

최종 상태 필드는 다음이면 충분하다.

```cpp
UPROPERTY(Transient)
TObjectPtr<UMAStreamingLevelLoader> LevelLoader;

EPhase Phase = EPhase::Idle;
FMASpaceTransitionRequest ActiveRequest;

TWeakObjectPtr<AMALevelRoot> CurrentLevel;
TWeakObjectPtr<AMALevelRoot> DestinationLevel;

TSet<TWeakObjectPtr<AMAPlayerControllerBase>> PendingPlayers;
```

각 역할:

```text
LevelLoader
= Streaming Level load/unload 구현

Phase
= 현재 Transition의 진행 단계

ActiveRequest
= 현재 Destination을 만들기 위해 필요한 전환 데이터

CurrentLevel
= 현재 활성 Streaming Level의 대표 LevelRoot

DestinationLevel
= 현재 전환에서 로드된 Destination LevelRoot

PendingPlayers
= 현재 Phase에서 서버가 응답을 기다리는 PlayerController 집합
```

이보다 별도의 Transition 상태를 추가하지 않는다.

---

# 3. 유지하는 Phase

```cpp
enum class EPhase : uint8
{
    Idle,
    Loading,
    Closing,
    Opening
};
```

각 Phase 의미:

```text
Idle
= 전환 없음

Loading
= Destination Streaming Level 준비 중

Closing
= Source Transition Presentation을 닫는 중

Opening
= Player Handoff 후 Destination Presentation을 여는 중
```

서버는 이 `Phase` 자체로 현재 Client Progress 응답의 의미를 판단한다.

별도 Milestone enum을 두지 않는다.

---

# 4. 완전히 제거하는 개념

다음 항목은 최종 구조에 존재하지 않는다.

```text
FMASpaceTransitionRequest::RequestId
UMASpaceTransitionSubsystem::NextRequestId
UMASpaceTransitionSubsystem::LastAcceptedRequestId
UMASpaceTransitionSubsystem::IsActiveRequest()

EMASpaceTransitionClientMilestone

LocalSourceLevel
LocalDestinationLevel

GetLocalPlayerController()

Transition Queue
Transition Registry
Transition History
Instance Counter
Retry State
사용자 Cancel State
Commit 이후 Rollback State
```

---

# 5. RequestId 제거

## 5.1 제거 이유

한 번에 하나의 Transition만 실행되므로 다음 지속 상태는 필요 없다.

```cpp
int32 RequestId;
int32 NextRequestId;
int32 LastAcceptedRequestId;
```

기존 개념:

```text
Transition 0
Transition 1
Transition 2
...
```

최종 개념:

```text
현재 Transition이 있는가?
없으면 Idle
있으면 ActiveRequest + Phase
```

과거 Transition 번호를 기억하지 않는다.

---

# 6. DestinationInstanceIdentity

## 6.1 이 값은 RequestId가 아니다

`DestinationInstanceIdentity`는 별도 Transition 세대 번호가 아니다.

실제로 서버와 클라이언트가 동일한 Destination Streaming Instance를 생성하기 위한 Identity다.

```cpp
UPROPERTY()
FString DestinationInstanceIdentity;
```

Transition 시작 시 한 번 생성한다.

```cpp
ActiveRequest.DestinationInstanceIdentity = FString::Printf(
    TEXT("MA_Level_%s"),
    *FGuid::NewGuid().ToString(EGuidFormats::Digits));
```

예:

```text
MA_Level_11C38FD4B15A4CB0A848852C65F68F71
```

---

## 6.2 Identity 수명

```text
RequestTransition
 ↓
DestinationInstanceIdentity 생성
 ↓
ActiveRequest에 저장
 ↓
서버 Destination Load
 ↓
ClientPrepareSpaceTransition(ActiveRequest)
 ↓
클라이언트도 같은 Identity로 Destination Load
 ↓
Transition 진행
 ↓
ResetTransitionState
 ↓
ActiveRequest 초기화
 ↓
Identity도 사라짐
```

다음은 만들지 않는다.

```text
NextInstanceId
LastInstanceId
InstanceCounter
InstanceHistory
InstanceRegistry
```

---

## 6.3 Progress RPC 검증에도 같은 Identity를 재사용한다

이전 Transition이 Abort된 뒤 이미 송신된 Client Progress RPC가 늦게 도착할 수 있다.

서버가 과거 Transition 상태를 보관할 필요는 없다.

Client가 현재 `ActiveRequest.DestinationInstanceIdentity`를 응답에 같이 보내고,
서버는 현재 Transition Identity와 같은지만 확인한다.

```cpp
UFUNCTION(Server, Reliable)
void ServerNotifySpaceTransitionProgress(
    const FString& DestinationInstanceIdentity,
    bool bSucceeded);
```

서버 검증:

```cpp
if (DestinationInstanceIdentity != ActiveRequest.DestinationInstanceIdentity) return;
```

예:

```text
Transition A = ABC
Transition A Abort

Transition B = XYZ

A의 늦은 응답 = ABC
현재 ActiveRequest = XYZ

ABC != XYZ
→ 폐기
```

이 방식은 과거 Transition을 기억하지 않는다.

늦게 도착한 RPC가 자신이 어느 Destination에 대한 응답인지 들고 올 뿐이다.

---

# 7. FMASpaceTransitionRequest 최종 구조

```cpp
USTRUCT()
struct FMASpaceTransitionRequest
{
    GENERATED_BODY()

    UPROPERTY()
    TSoftObjectPtr<UWorld> DestinationMap;

    UPROPERTY()
    FTransform DestinationSlotTransform = FTransform::Identity;

    UPROPERTY()
    FString DestinationInstanceIdentity;

    UPROPERTY()
    int32 GenerationSeed = 0;

    bool IsValid() const
    {
        return !DestinationMap.IsNull() && !DestinationInstanceIdentity.IsEmpty();
    }
};
```

`GenerationSeed`는 유지한다.

Battle Level Runtime Generation은 확정된 후속 범위이므로,
서버와 클라이언트가 동일한 Seed를 공유할 수 있는 기반은 현재 설계 범위에 포함한다.

`IsValid()`도 유지한다.

이 함수는 여러 전환 진입점에서 반복되는 최소 Request 불변식 검사를 한 곳에 둔다.

---

# 8. Milestone 제거

다음 enum은 제거한다.

```cpp
UENUM()
enum class EMASpaceTransitionClientMilestone : uint8
{
    DestinationReady,
    Closed,
    Opened
};
```

이 정보는 이미 서버 `Phase`에 존재한다.

```text
Phase == Loading
→ 지금 들어오는 Progress는 Destination Ready 결과

Phase == Closing
→ 지금 들어오는 Progress는 Close 결과

Phase == Opening
→ 지금 들어오는 Progress는 Open 결과
```

Client가 같은 의미를 네트워크로 다시 보낼 필요가 없다.

최종 Progress 정보:

```text
DestinationInstanceIdentity
= 어느 현재 Transition의 응답인가

PlayerController
= 어떤 Player의 응답인가

bSucceeded
= 성공했는가

Phase
= 서버가 현재 어떤 단계의 응답을 기다리는가
```

---

# 9. PlayerController 책임

`AMAPlayerControllerBase`는 Space Transition의 네트워크 경계만 담당한다.

정책과 상태를 가지지 않는다.

```text
Server Transition Subsystem
        ↓
PlayerController Client RPC
        ↓
Client Transition Subsystem
```

반대 방향:

```text
Client Transition Subsystem
        ↓
PlayerController Server RPC
        ↓
Server Transition Subsystem
```

PC 내부에 다음을 추가하지 않는다.

```text
ActiveTransition
TransitionPhase
DestinationLevel
DestinationInstanceIdentity 저장 필드
Pending State
Transition Policy
```

---

# 10. PlayerController 최종 RPC

Server -> Client:

```cpp
UFUNCTION(Client, Reliable)
void ClientPrepareSpaceTransition(
    const FMASpaceTransitionRequest& Request);

UFUNCTION(Client, Reliable)
void ClientCloseSpaceTransition();

UFUNCTION(Client, Reliable)
void ClientCommitSpaceTransition();

UFUNCTION(Client, Reliable)
void ClientAbortSpaceTransition();
```

Client -> Server:

```cpp
UFUNCTION(Server, Reliable)
void ServerNotifySpaceTransitionProgress(
    const FString& DestinationInstanceIdentity,
    bool bSucceeded);
```

PC 구현은 전달만 한다.

예:

```cpp
void AMAPlayerControllerBase::ClientCloseSpaceTransition_Implementation()
{
    if (UMASpaceTransitionSubsystem* SpaceTransition =
        GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
    {
        SpaceTransition->BeginLocalClose();
    }
}
```

별도 Transition Network Component는 만들지 않는다.

---

# 11. Cancel -> Abort

`Cancel`은 사용자 기능처럼 읽히지만,
실제 동작은 Commit 이전 실패로 인해 현재 Transition을 중단하는 것이다.

따라서 다음 이름을 사용한다.

```text
CancelTransition
→ AbortTransition

CancelLocalTransition
→ AbortLocalTransition

ClientCancelSpaceTransition
→ ClientAbortSpaceTransition
```

사용자가 임의로 Transition을 취소하는 기능은 만들지 않는다.

---

# 12. RequestTransition

서버 `RequestTransition()` 책임:

```text
서버 권한 확인
Idle 확인
CurrentLevel 확인
Current TransitionCircle 확인
DestinationMap 확인
DestinationInstanceIdentity 생성
ActiveRequest 구성
Phase = Loading
remote Client를 PendingPlayers에 등록
Destination Load 시작
ClientPrepareSpaceTransition(Request) 전송
```

현재 코드의 방향을 유지한다.

핵심 상태 변경:

```cpp
ActiveRequest.DestinationMap = DestinationMap;
ActiveRequest.DestinationSlotTransform = DestinationSlotTransform;
ActiveRequest.DestinationInstanceIdentity = FString::Printf(
    TEXT("MA_Level_%s"),
    *FGuid::NewGuid().ToString(EGuidFormats::Digits));
ActiveRequest.GenerationSeed = GenerationSeed;
Phase = EPhase::Loading;
```

별도 Request 세대 상태는 만들지 않는다.

---

# 13. BeginClientPrepare

Pure Client는 Idle일 때만 Prepare를 받아들인다.

```cpp
void UMASpaceTransitionSubsystem::BeginClientPrepare(
    const FMASpaceTransitionRequest& Request)
{
    if (!Request.IsValid() || Phase != EPhase::Idle) return;

    ActiveRequest = Request;
    Phase = EPhase::Loading;

    if (!LoadDestination(Request))
    {
        NotifyServer(false);
    }
}
```

다음 세대 비교는 존재하지 않는다.

```text
RequestId 비교
LastAcceptedRequestId 비교
과거 Request 저장
```

새 Prepare가 들어왔다는 이유로 기존 Transition을 강제로 Reset하지 않는다.

동시에 두 Transition을 보내지 않는 것이 시스템 계약이다.

---

# 14. LoadDestination

Loader는 전달받은 Destination 정보를 Streaming 구현에만 사용한다.

```cpp
return LevelLoader->LoadLevel(
    Request.DestinationMap,
    Request.DestinationSlotTransform,
    Request.DestinationInstanceIdentity,
    FOnMALevelLoaded::CreateUObject(
        this,
        &UMASpaceTransitionSubsystem::HandleDestinationLoaded));
```

`UMAStreamingLevelLoader`는 Identity 생성 규칙을 알지 않는다.

Transition Subsystem이 현재 Transition용 Identity를 만들고,
Loader는 그 값을 사용한다.

---

# 15. Destination Load 완료

`HandleDestinationLoaded()` 책임:

```text
Loading Phase인지 확인
DestinationLevel 저장
Destination LevelRoot 유효성 확인
Destination TransitionCircle 확인
Client면 서버에 결과 통지
Server면 모든 Client Ready 이후 Close 단계 진입
```

Client:

```text
Load 성공
→ NotifyServer(true)

Load 실패
→ NotifyServer(false)
```

Server:

```text
Destination 준비 성공
→ TryBeginClose()

Destination 준비 실패
→ AbortTransition()
```

Milestone은 필요 없다.

현재 `Phase == Loading`이 응답 의미를 이미 정의한다.

---

# 16. LocalSourceLevel / LocalDestinationLevel 제거

다음 복제 상태는 제거한다.

```cpp
TWeakObjectPtr<AMALevelRoot> LocalSourceLevel;
TWeakObjectPtr<AMALevelRoot> LocalDestinationLevel;
```

기존에는 Close 시점에 다음처럼 같은 Level을 다시 저장했다.

```cpp
LocalSourceLevel = CurrentLevel;
LocalDestinationLevel = DestinationLevel;
```

하지만 최종 구조에서는 Destination Open 완료 전까지 `PromoteDestination()`을 하지 않는다.

따라서 Opening이 끝날 때까지 항상 다음 관계가 유지된다.

```text
CurrentLevel
= Source

DestinationLevel
= Destination
```

Handoff, Abort, Open 완료 처리 모두 직접 이 둘을 사용한다.

별도 Local 복제 참조는 필요 없다.

---

# 17. BeginLocalClose Phase 계약

최종 시그니처:

```cpp
void BeginLocalClose();
```

RequestId를 받지 않는다.

Pure Client의 정상 진입 Phase:

```text
Loading
```

Listen Server / Standalone에서는 서버와 로컬 PlayerController가 같은 `UWorldSubsystem`을 공유한다.

서버 `TryBeginClose()`가 먼저:

```cpp
Phase = EPhase::Closing;
```

으로 바꾼 뒤 로컬 `ClientCloseSpaceTransition()`이 같은 Subsystem으로 들어올 수 있다.

따라서 현재 구현의 Phase 허용은 유지한다.

```cpp
const bool bExpectedPhase =
    Phase == EPhase::Loading ||
    (Phase == EPhase::Closing && GetWorld()->GetNetMode() != NM_Client);
```

이 예외는 별도 상태가 아니라 Listen Server의 동일 World/Subsystem 실행 특성을 반영한 것이다.

Close 성공:

```text
SourceCircle->CloseTransition
 ↓
HandleLocalCloseFinished
 ↓
NotifyServer(true)
```

Close 시작 실패:

```text
NotifyServer(false)
```

---

# 18. TryBeginClose

서버가 다음 조건을 모두 만족할 때 Closing을 시작한다.

```text
Server
Phase == Loading
DestinationLevel valid
PendingPlayers empty
```

`PendingPlayers`가 비어 있다는 것은:

```text
서버 Destination 준비 완료
+ 모든 remote Client DestinationReady 응답 완료
```

를 의미한다.

그 뒤:

```text
Phase = Closing
 ↓
현재 PlayerController 전부 PendingPlayers에 등록
 ↓
ClientCloseSpaceTransition()
```

단계 진입 직전 `PendingPlayers`가 이미 비어 있음이 조건으로 보장되므로,
불필요한 `PendingPlayers.Reset()`은 두지 않는다.

---

# 19. BeginLocalHandoff Phase 계약

최종 시그니처:

```cpp
void BeginLocalHandoff();
```

Pure Client 정상 진입 Phase:

```text
Closing
```

Listen Server / Standalone에서는 서버가 `CommitHandoff()`에서 먼저:

```cpp
Phase = EPhase::Opening;
```

으로 바꾼 뒤 로컬 `ClientCommitSpaceTransition()`이 같은 Subsystem으로 들어올 수 있다.

따라서 현재 구현의 Phase 허용은 유지한다.

```cpp
const bool bExpectedPhase =
    Phase == EPhase::Closing ||
    (Phase == EPhase::Opening && GetWorld()->GetNetMode() != NM_Client);
```

Handoff Presentation 순서:

```text
SourceCircle ReleaseTransitionPresentation
 ↓
DestinationCircle SetTransitionClosed
 ↓
Phase = Opening
 ↓
DestinationCircle OpenTransition
```

---

# 20. Client Promote 시점

Client는 Destination Open 시작 전에 Source를 unload하지 않는다.

잘못된 순서:

```text
Destination Closed
 ↓
PromoteDestination
 ↓
Source unload
 ↓
Destination Open
```

최종 순서:

```text
Destination Closed
 ↓
Destination Open
 ↓
Open 완료 callback
 ↓
CompleteLocalHandoff
 ↓
PromoteDestination
 ↓
Source unload
```

이렇게 하면 Server와 Client 모두 같은 불변 규칙을 가진다.

> Destination Open 처리가 완료된 뒤 Source Streaming Instance를 제거하고 Destination을 CurrentLevel로 승격한다.

---

# 21. CommitHandoff

서버 Commit은 Player 위치를 실제 Destination으로 옮기는 되돌릴 수 없는 경계다.

```text
CurrentLevel / DestinationLevel 확인
 ↓
MovePlayersToDestination
 ↓
Phase = Opening
 ↓
현재 PlayerController 전부 PendingPlayers 등록
 ↓
ClientCommitSpaceTransition()
```

이 시점 이후에는 Source로 Rollback하는 시스템을 만들지 않는다.

`PendingPlayers`가 이전 Closing 단계 완료 시 이미 비어 있으므로,
여기서도 불필요한 Reset을 추가하지 않는다.

---

# 22. HandleClientProgress 최종 정리

이번 추가 정리의 핵심이다.

현재 구현에는 다음 중복이 있다.

```cpp
!PendingPlayers.Contains(&PlayerController)
```

으로 먼저 검사하고,
이후 다시:

```cpp
PendingPlayers.Remove(&PlayerController);
```

를 수행한다.

`TSet::Remove()` 반환값이 바로 "기다리던 Player였는가"를 알려주므로 둘을 하나로 합친다.

또한 `Phase == Idle`이면 `ResetTransitionState()`에 의해 `ActiveRequest`가 이미 invalid여야 한다.

따라서 다음 별도 Guard도 중복이다.

```cpp
if (Phase == EPhase::Idle) return;
```

최종 형태:

```cpp
void UMASpaceTransitionSubsystem::HandleClientProgress(
    AMAPlayerControllerBase& PlayerController,
    const FString& DestinationInstanceIdentity,
    const bool bSucceeded)
{
    if (GetWorld()->GetNetMode() == NM_Client ||
        !ActiveRequest.IsValid() ||
        DestinationInstanceIdentity != ActiveRequest.DestinationInstanceIdentity)
    {
        return;
    }

    if (PendingPlayers.Remove(&PlayerController) == 0) return;

    if (!bSucceeded)
    {
        if (Phase == EPhase::Opening)
        {
            UE_LOG(
                LogMASpaceTransition,
                Error,
                TEXT("A client failed to open Space transition '%s'."),
                *DestinationInstanceIdentity);

            if (PendingPlayers.IsEmpty()) FinishTransition();
        }
        else
        {
            AbortTransition();
        }
        return;
    }

    switch (Phase)
    {
    case EPhase::Loading:
        TryBeginClose();
        break;

    case EPhase::Closing:
        if (PendingPlayers.IsEmpty()) CommitHandoff();
        break;

    case EPhase::Opening:
        if (PendingPlayers.IsEmpty()) FinishTransition();
        break;

    default:
        break;
    }
}
```

핵심 의미:

```text
Identity 검사
= 현재 Transition의 응답인가

PendingPlayers.Remove(...) != 0
= 현재 서버가 실제로 기다리던 Player의 첫 응답인가

Phase
= 그 응답이 어떤 단계의 결과인가

bSucceeded
= 그 단계가 성공했는가
```

별도 Milestone, RequestId, 중복 Contains 검사는 필요 없다.

---

# 23. 한 줄 Guard 스타일

짧은 단일 Guard는 기존 Transition 코드 스타일에 맞춰 중괄호 없이 한 줄로 둔다.

권장:

```cpp
if (PendingPlayers.Remove(&PlayerController) == 0) return;
if (!LevelLoader) return false;
if (!Request.IsValid() || Phase != EPhase::Idle) return;
```

불필요하게 다음처럼 늘리지 않는다.

```cpp
if (PendingPlayers.Remove(&PlayerController) == 0)
{
    return;
}
```

단, 여러 줄의 복합 조건이나 본문에 여러 문장이 있는 경우에는 중괄호를 유지한다.

예:

```cpp
if (GetWorld()->GetNetMode() == NM_Client ||
    !ActiveRequest.IsValid() ||
    DestinationInstanceIdentity != ActiveRequest.DestinationInstanceIdentity)
{
    return;
}
```

목표는 무조건 한 줄로 압축하는 것이 아니라,
단순 Guard는 단순하게 보이도록 하는 것이다.

---

# 24. PendingPlayers 책임

`PendingPlayers`는 유지한다.

단순 `int32 PendingCount`로 바꾸지 않는다.

이유:

```text
어떤 Player의 응답을 기다리는지 알아야 함
같은 Player의 중복 응답을 다시 소비하면 안 됨
현재 Transition에 포함되지 않은 Player의 응답을 무시해야 함
```

최종 소비 방식:

```cpp
if (PendingPlayers.Remove(&PlayerController) == 0) return;
```

이 한 줄이 다음 두 역할을 동시에 수행한다.

```text
멤버십 확인
+ 응답 소비
```

---

# 25. AbortLocalTransition

Local Abort는 Commit 이전에만 허용한다.

```text
Loading
Closing
```

Opening은 이미 Commit 이후이므로 Abort 대상이 아니다.

Closing 중 Source가 닫혀 있었다면 다시 연다.

```cpp
if (Phase == EPhase::Closing)
{
    if (AMAMagicCircle* SourceCircle = CurrentLevel.IsValid()
        ? CurrentLevel->GetTransitionCircle()
        : nullptr)
    {
        SourceCircle->OpenTransition();
    }
}
```

Pure Client는:

```text
DiscardDestination
 ↓
ResetTransitionState
```

Server / Listen Server의 shared Subsystem은 서버 `AbortTransition()`이 최종 정리를 담당한다.

---

# 26. AbortTransition

Server Abort 순서:

```text
각 PlayerController에 ClientAbortSpaceTransition
 ↓
DiscardDestination
 ↓
ResetTransitionState
```

Abort는 Commit 이전 실패 경로다.

예:

```text
Server Destination Load 실패
Client Destination Load 실패
Destination LevelRoot/Circle 검증 실패
Source Close 실패
Client Close 실패
```

사용자 취소 기능이 아니다.

---

# 27. Commit 이후 Open 실패

Opening은 이미 Player Handoff가 Commit된 뒤다.

따라서 Open 실패 때문에 Source로 Rollback하지 않는다.

Client Open 실패:

```text
Source/Destination Presentation 정리
 ↓
Destination Promote
 ↓
Source unload
 ↓
NotifyServer(false)
 ↓
Client ResetTransitionState
```

Server:

```text
해당 Player 응답 소비
 ↓
다른 Pending Player 응답 계속 대기
 ↓
PendingPlayers empty
 ↓
FinishTransition
```

다음 기능은 만들지 않는다.

```text
Player 위치 저장
Source 재활성화
Commit Rollback
Streaming 복구
Retry State Machine
```

---

# 28. CompleteLocalHandoff

`CompleteLocalHandoff(bool bSucceeded)`는 유지한다.

이 함수는 단순 callback wrapper가 아니라 다음 책임을 한 곳에서 처리한다.

```text
Open 실패 시 Presentation 정리
Pure Client Destination Promote
서버 Progress 통지
Pure Client Transition State Reset
```

따라서 제거하지 않는다.

Open 성공:

```text
HandleLocalOpenFinished
 ↓
CompleteLocalHandoff(true)
```

Open 시작 실패:

```text
CompleteLocalHandoff(false)
```

---

# 29. HandleLocalCloseFinished / HandleLocalOpenFinished

다음 callback 함수는 유지한다.

```cpp
void HandleLocalCloseFinished();
void HandleLocalOpenFinished();
```

내용은 짧지만 Delegate callback의 의미를 이름으로 명확히 드러낸다.

```cpp
void UMASpaceTransitionSubsystem::HandleLocalCloseFinished()
{
    NotifyServer(true);
}

void UMASpaceTransitionSubsystem::HandleLocalOpenFinished()
{
    CompleteLocalHandoff(true);
}
```

이를 억지로 Lambda/Payload binding에 합쳐 코드 의미를 숨기지 않는다.

---

# 30. PromoteDestination 책임 정리

현재 구현은 `PromoteDestination()` 안에서:

```cpp
DestinationLevel.Reset();
```

까지 수행하고,
직후 `ResetTransitionState()`에서 다시 같은 Reset을 한다.

최종 구조에서는 책임을 분리한다.

`PromoteDestination()` 책임:

```text
Source Streaming Level unload
Destination을 CurrentLevel로 승격
```

최종 형태:

```cpp
void UMASpaceTransitionSubsystem::PromoteDestination()
{
    AMALevelRoot* Source = CurrentLevel.Get();
    AMALevelRoot* Destination = DestinationLevel.Get();
    if (!ensure(Source && Destination && LevelLoader)) return;

    LevelLoader->UnloadLevel(*Source);
    CurrentLevel = Destination;
}
```

`DestinationLevel` 참조 제거는 `ResetTransitionState()`가 담당한다.

---

# 31. DiscardDestination 책임 정리

현재 구현은 `DiscardDestination()` 안에서도:

```cpp
DestinationLevel.Reset();
```

을 수행하고,
직후 `ResetTransitionState()`가 다시 같은 Reset을 한다.

최종 구조에서는 중복을 제거한다.

`DiscardDestination()` 책임:

```text
로드된 Destination Streaming Level이 있으면 unload
진행 중인 pending load cancel
```

최종 형태:

```cpp
void UMASpaceTransitionSubsystem::DiscardDestination()
{
    if (!LevelLoader) return;

    if (AMALevelRoot* Destination = DestinationLevel.Get())
    {
        LevelLoader->UnloadLevel(*Destination);
    }

    LevelLoader->CancelPendingLoad();
}
```

`DestinationLevel` 참조 제거는 `ResetTransitionState()` 하나가 담당한다.

---

# 32. ResetTransitionState가 유일한 Transition 상태 정리 지점

최종:

```cpp
void UMASpaceTransitionSubsystem::ResetTransitionState()
{
    Phase = EPhase::Idle;
    ActiveRequest = FMASpaceTransitionRequest();
    DestinationLevel.Reset();
    PendingPlayers.Reset();
}
```

이 함수가 Transition 전용 상태를 최종적으로 비우는 유일한 지점이다.

책임 구분:

```text
PromoteDestination
= Current Level 교체

DiscardDestination
= Destination Streaming 폐기

ResetTransitionState
= Transition Runtime 상태 제거
```

`CurrentLevel`은 Reset하지 않는다.

---

# 33. FinishTransition

정상 완료:

```cpp
void UMASpaceTransitionSubsystem::FinishTransition()
{
    UE_LOG(
        LogMASpaceTransition,
        Log,
        TEXT("Transition '%s' completed."),
        *ActiveRequest.DestinationInstanceIdentity);

    PromoteDestination();
    ResetTransitionState();
}
```

책임:

```text
Destination Promote
 ↓
Transition State 제거
```

`PromoteDestination()`은 State Reset을 하지 않는다.

---

# 34. NotifyServer

별도 `GetLocalPlayerController()` helper는 만들지 않는다.

현재처럼 `NotifyServer()` 안에서 직접 local PlayerController를 얻는다.

```cpp
void UMASpaceTransitionSubsystem::NotifyServer(const bool bSucceeded)
{
    if (!ActiveRequest.IsValid()) return;

    if (AMAPlayerControllerBase* PlayerController =
        Cast<AMAPlayerControllerBase>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        PlayerController->ServerNotifySpaceTransitionProgress(
            ActiveRequest.DestinationInstanceIdentity,
            bSucceeded);
    }
}
```

한 곳에서만 쓰는 한 줄짜리 Getter abstraction을 다시 만들지 않는다.

---

# 35. MovePlayersToDestination

현재 책임을 유지한다.

```text
Source TransitionCircle 기준 Player 상대 Transform 계산
 ↓
Destination TransitionCircle 기준 World Transform 복원
 ↓
Pawn TeleportPhysics 이동
```

Pawn/Controller를 재생성하지 않는다.

PlayerStart 기반 재배치도 사용하지 않는다.

```cpp
const FTransform RelativeTransform =
    SourceCircle->WorldToCircleTransform(Pawn->GetActorTransform());

const FTransform DestinationTransform =
    DestinationCircle->CircleToWorldTransform(RelativeTransform);
```

이 기능은 Transition Subsystem의 Handoff 책임에 포함된다.

---

# 36. Normal Transition 전체 순서

## Server

```text
RequestTransition
 ↓
ActiveRequest 생성
DestinationInstanceIdentity 생성
Phase = Loading
 ↓
Server Destination Load
remote Client Prepare
 ↓
모든 remote Client Ready
 ↓
TryBeginClose
Phase = Closing
 ↓
모든 Player Close
 ↓
CommitHandoff
Player 이동
Phase = Opening
 ↓
모든 Player Destination Open
 ↓
FinishTransition
 ↓
PromoteDestination
Source unload
 ↓
ResetTransitionState
 ↓
Idle
```

## Pure Client

```text
ClientPrepareSpaceTransition
 ↓
ActiveRequest 저장
Phase = Loading
Destination Load
 ↓
NotifyServer(true)
 ↓
ClientCloseSpaceTransition
Phase = Closing
Source Close
 ↓
NotifyServer(true)
 ↓
ClientCommitSpaceTransition
Destination Closed
Phase = Opening
Destination Open
 ↓
CompleteLocalHandoff
 ↓
PromoteDestination
Source unload
 ↓
NotifyServer(true)
ResetTransitionState
 ↓
Idle
```

---

# 37. Abort 전체 순서

Commit 이전 실패:

```text
Loading 또는 Closing 실패
 ↓
Server AbortTransition
 ↓
ClientAbortSpaceTransition 전송
 ↓
Client Source Presentation 복구
Client Destination Streaming 폐기
Client ResetTransitionState
 ↓
Server Destination Streaming 폐기
Server ResetTransitionState
 ↓
Idle
```

과거 Transition 상태는 남지 않는다.

늦은 Progress RPC는 해당 RPC가 들고 온 이전 `DestinationInstanceIdentity`가 현재 `ActiveRequest`와 다르므로 폐기된다.

---

# 38. Listen Server / Standalone 공유 Subsystem 주의사항

Listen Server의 local PlayerController RPC는 다른 Client처럼 별도 Client World의 Subsystem으로 들어가는 것이 아니다.

서버와 같은 `UWorldSubsystem` 상태를 공유한다.

따라서 다음 순서가 가능하다.

Closing:

```text
Server TryBeginClose
Phase = Closing
 ↓
local ClientCloseSpaceTransition
 ↓
같은 Subsystem BeginLocalClose
```

Opening:

```text
Server CommitHandoff
Phase = Opening
 ↓
local ClientCommitSpaceTransition
 ↓
같은 Subsystem BeginLocalHandoff
```

그래서 `BeginLocalClose()`와 `BeginLocalHandoff()`의 non-client Phase 예외는 유지한다.

이 예외를 제거하고 Pure Client 규칙만 적용하면 Listen Server local transition이 실패할 수 있다.

---

# 39. 명시적으로 유지하는 것

다음 요소는 현재 구조에서 실제 역할이 있으므로 유지한다.

```text
LevelLoader
Phase
ActiveRequest
CurrentLevel
DestinationLevel
PendingPlayers

DestinationMap
DestinationSlotTransform
DestinationInstanceIdentity
GenerationSeed
IsValid()

LoadDestination
HandleDestinationLoaded
TryBeginClose
CommitHandoff
FinishTransition
AbortTransition
DiscardDestination
ResetTransitionState

HandleLocalCloseFinished
HandleLocalOpenFinished
CompleteLocalHandoff
NotifyServer

MovePlayersToDestination
PromoteDestination
```

단순히 코드 줄 수를 줄이기 위해 역할이 명확한 함수까지 합치지 않는다.

---

# 40. 명시적으로 추가하지 않는 것

```text
RequestId
TransitionId
NextRequestId
LastAcceptedRequestId
Milestone enum

LocalSourceLevel
LocalDestinationLevel

NextInstanceId
InstanceCounter
InstanceHistory

Transition Queue
Transition Registry
Transition Context UObject
별도 Transition Network Component

사용자 Cancel 기능
Retry 시스템
Commit 이후 Rollback 시스템
Player 위치 Snapshot 시스템
```

실제 필요가 확인되기 전에는 추가하지 않는다.

---

# 41. 변경 파일 범위

이 리팩터의 핵심 수정 대상:

```text
Source/P_MA/Private/Level/Transition/MASpaceTransitionTypes.h

Source/P_MA/Private/Level/Transition/MASpaceTransitionSubsystem.h
Source/P_MA/Private/Level/Transition/MASpaceTransitionSubsystem.cpp

Source/P_MA/Private/Player/MAPlayerControllerBase.h
Source/P_MA/Private/Player/MAPlayerControllerBase.cpp
```

이번 최종 군더더기 정리에서 실제 추가 수정이 필요한 핵심 파일은:

```text
Source/P_MA/Private/Level/Transition/MASpaceTransitionSubsystem.cpp
```

현재 `Types`와 `PlayerController`는 이미 목표 구조에 가까우므로,
이번 마지막 정리를 이유로 불필요하게 다시 확장하지 않는다.

다음 파일은 이번 Runtime State 정리를 이유로 변경하지 않는다.

```text
MAMagicCircle.h
MAMagicCircle.cpp

MASpaceTransitionMaskComponent.h
MASpaceTransitionMaskComponent.cpp

MASpaceTransitionVisibilityComponent.h
MASpaceTransitionVisibilityComponent.cpp

MALevelRoot.h
MAStreamingLevelLoader.h
MAStreamingLevelLoader.cpp
```

---

# 42. 이번 마지막 구현 수정 항목

현재 구현에서 추가로 적용할 것은 정확히 다음이다.

```text
1. HandleClientProgress의 PendingPlayers.Contains 제거
2. PendingPlayers.Remove 반환값으로 membership 확인 + 응답 소비 통합
3. 한 줄 Guard로 작성
4. HandleClientProgress의 별도 Phase == Idle Guard 제거
5. PromoteDestination의 DestinationLevel.Reset 제거
6. DiscardDestination의 DestinationLevel.Reset 제거
7. DestinationLevel.Reset 책임을 ResetTransitionState 하나로 통일
```

그 외 구조를 다시 뜯지 않는다.

---

# 43. 구현 순서

## Step 1 - HandleClientProgress 단순화

기존 개념:

```cpp
if (... || !PendingPlayers.Contains(&PlayerController))
{
    return;
}

if (Phase == EPhase::Idle) return;

...

PendingPlayers.Remove(&PlayerController);
```

변경:

```cpp
if (GetWorld()->GetNetMode() == NM_Client ||
    !ActiveRequest.IsValid() ||
    DestinationInstanceIdentity != ActiveRequest.DestinationInstanceIdentity)
{
    return;
}

if (PendingPlayers.Remove(&PlayerController) == 0) return;
```

이후 함수 안의 별도 `PendingPlayers.Remove()`를 삭제한다.

Opening 실패 경로에서도 이미 응답이 소비된 상태이므로 별도 Remove를 하지 않는다.

---

## Step 2 - PromoteDestination 책임 축소

삭제:

```cpp
DestinationLevel.Reset();
```

남김:

```cpp
LevelLoader->UnloadLevel(*Source);
CurrentLevel = Destination;
```

---

## Step 3 - DiscardDestination 책임 축소

삭제:

```cpp
DestinationLevel.Reset();
```

남김:

```text
Destination unload
Pending Load cancel
```

---

## Step 4 - ResetTransitionState를 유일한 State cleanup으로 유지

```cpp
Phase = EPhase::Idle;
ActiveRequest = FMASpaceTransitionRequest();
DestinationLevel.Reset();
PendingPlayers.Reset();
```

---

## Step 5 - 검색 검수

다음 문자열을 전체 Transition 범위에서 검색한다.

```text
RequestId
NextRequestId
LastAcceptedRequestId
IsActiveRequest
EMASpaceTransitionClientMilestone
LocalSourceLevel
LocalDestinationLevel
GetLocalPlayerController
```

의도한 Runtime 코드에 남아 있지 않아야 한다.

---

# 44. 구현 후 검수 기준

## State

```text
[ ] LevelLoader만 Loader 수명 상태
[ ] CurrentLevel만 Transition 사이에서 유지
[ ] Phase / ActiveRequest / DestinationLevel / PendingPlayers만 현재 Transition 상태
[ ] RequestId 계열 없음
[ ] Milestone 없음
[ ] Local Level 복제 상태 없음
```

## Networking

```text
[ ] Prepare RPC만 FMASpaceTransitionRequest 전달
[ ] Close RPC에 별도 ID 없음
[ ] Commit RPC에 별도 ID 없음
[ ] Abort RPC에 별도 ID 없음
[ ] Progress RPC는 DestinationInstanceIdentity + bSucceeded만 전달
[ ] PlayerController는 RPC forwarding만 담당
```

## Progress

```text
[ ] 현재 Identity가 아니면 무시
[ ] PendingPlayers.Remove 반환값으로 기다리던 Player인지 확인
[ ] 같은 Player 응답은 한 번만 소비
[ ] 별도 PendingPlayers.Contains 없음
[ ] 별도 Phase == Idle Guard 없음
[ ] 서버 Phase가 응답 의미를 결정
```

## Cleanup

```text
[ ] PromoteDestination은 CurrentLevel 교체만 담당
[ ] DiscardDestination은 Destination streaming 폐기만 담당
[ ] ResetTransitionState만 Transition 참조를 최종 Reset
[ ] DestinationLevel.Reset 중복 없음
```

## Presentation / Lifecycle

```text
[ ] Destination 준비 후 Source Close
[ ] Source/Destination Transition Presentation handoff 유지
[ ] Client Source unload는 Destination Open 처리 뒤
[ ] Commit 이전 실패만 Abort
[ ] Commit 이후 Open 실패는 Destination으로 완료
```

## World / Player

```text
[ ] Persistent UWorld 유지
[ ] ServerTravel 사용하지 않음
[ ] Pawn 재생성 없음
[ ] Controller 재생성 없음
[ ] Circle 상대 Transform 유지
```

## Style

```text
[ ] 짧은 단일 Guard는 한 줄
[ ] 여러 줄 복합 조건은 중괄호 유지
[ ] 의미 없는 helper 추가 없음
[ ] 중복 상태 추가 없음
```

---

# 45. 최종 상태 그림

```text
UMASpaceTransitionSubsystem

World Lifetime
│
├─ LevelLoader
├─ CurrentLevel
│
└─ Active Transition
   │
   ├─ Phase
   │
   ├─ ActiveRequest
   │  ├─ DestinationMap
   │  ├─ DestinationSlotTransform
   │  ├─ DestinationInstanceIdentity
   │  └─ GenerationSeed
   │
   ├─ DestinationLevel
   └─ PendingPlayers
```

Transition 완료 후:

```text
UMASpaceTransitionSubsystem

LevelLoader
CurrentLevel = 이전 Destination
Phase = Idle
ActiveRequest = Empty
DestinationLevel = null
PendingPlayers = Empty
```

과거 Transition을 설명하기 위한 별도 상태는 남지 않는다.

---

# 최종 원칙

> Space Transition은 현재 진행 중인 전환 하나만 모델링한다.
> `DestinationInstanceIdentity`는 현재 Destination Streaming Instance를 식별하기 위해 존재하며,
> 같은 값을 늦은 Progress RPC 검증에도 재사용한다.
> 별도 RequestId, Milestone, Local Level 복제 상태를 두지 않는다.
> `PendingPlayers`는 응답 대상을 추적하면서 `Remove()`로 응답 소비까지 담당한다.
> Streaming 교체/폐기와 Runtime State Reset의 책임을 분리하고,
> Transition이 끝나면 현재 Transition 데이터 전체가 사라진다.
