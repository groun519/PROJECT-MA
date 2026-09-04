# 07. Stencil Ownership Refactor Mockup

> Status: Final implementation contract
> Target branch: feature/psw/level-system-rework
> Reviewed local-change base HEAD: 8b6e66e30e59ccaf6687035978f5d31097628903
> Mockup branch: codex/mockup-level-system-rework
> Engine baseline: Unreal Engine 5.8
> Supersedes: 7a608a7cef1fa69651309dad2a89f747f2a44dcd의 초기 Stencil ownership 계약

## 1. 목적

현재 `UMAHighlightComponent`와 `UMASpaceTransitionVisibilityComponent`가 같은 `UPrimitiveComponent`의 Custom Depth / Stencil 상태를 각각 직접 제어한다.

현재 Transition Visibility는 충돌을 피하기 위해 다음 상태를 snapshot으로 저장하고 복구한다.

~~~text
RenderCustomDepth
CustomDepthStencilValue
CustomDepthStencilWriteMask
Highlight 활성 상태
~~~

Player에서는 Transition Visibility가 `UMAHighlightComponent`를 직접 참조해 Transition 동안 Highlight를 비활성화하고 종료 후 다시 활성화한다.

이 구조는 두 기능이 실제로 공유하는 자원보다 서로의 구현을 더 많이 알게 만든다.

이번 수정의 목표는 Custom Stencil 8bit를 현재 존재하는 두 용도에 명시적으로 분할하고, 하나의 작은 stateless C++ 연산 타입이 실제 Primitive 상태 변경만 닫도록 만드는 것이다.

핵심 원칙:

> Custom Stencil 값 자체가 Highlight와 Transition의 현재 공유 상태다.
>
> Highlight는 하위 7bit만 소유한다.
>
> Transition Visibility는 최상위 1bit만 소유한다.
>
> 한 기능이 다른 기능의 bit를 덮어쓰지 않는다.
>
> Transition 활성 여부에 따라 `CustomDepthStencilWriteMask`도 공용 연산이 일관되게 설정한다.
>
> 최종 Stencil 값이 0이 아니면 `RenderCustomDepth`를 유지한다.
>
> Highlight와 Transition Visibility는 서로 직접 참조하지 않는다.
>
> 별도 Manager, Registry, Allocator, 상태 저장 UObject를 만들지 않는다.

## 2. 최종 Stencil 계약

Custom Stencil 8bit를 다음처럼 사용한다.

~~~text
bit 7        bit 6 ~ 0
Transition   Highlight
---------    ---------
0 / 1        0 ~ 127
~~~

내부 상수 의미:

~~~cpp
HighlightMask = 0x7F; // 01111111
TransitionBit = 0x80; // 10000000
~~~

현재 Highlight는 Hue 기반 1~36, White 37만 사용하므로 하위 7bit 범위 안에 충분히 들어온다.

예:

~~~text
Highlight 12 only
00001100 = 12

Transition only
10000000 = 128

Transition + Highlight 12
10001100 = 140
~~~

0은 Highlight 없음 + Transition 없음 상태다.

## 3. FMARenderStencil

두 Component가 bit 연산과 WriteMask / RenderCustomDepth 갱신을 중복 구현하지 않는다.

다음 작은 stateless C++ 타입을 추가한다.

~~~text
Source/P_MA/Private/MARenderStencil.h
~~~

최종 타입:

~~~cpp
struct FMARenderStencil
{
    static void SetHighlightValue(
        UPrimitiveComponent& Primitive,
        uint8 HighlightValue);

    static void SetTransitionVisible(
        UPrimitiveComponent& Primitive,
        bool bVisible);
};
~~~

외부 API는 실제 사용되는 두 함수만 둔다.

다음 public API는 만들지 않는다.

~~~text
GetHighlightValue()
HasTransition()
GetStencilState()
SetStencilState()
~~~

`HighlightMask`, `TransitionBit`과 최종 상태 적용에 필요한 세부 연산은 타입 내부 구현 세부사항이다.

`namespace MARenderStencil`은 사용하지 않는다.

별도 UObject, ActorComponent, Manager로 만들지 않는다.

## 4. 공용 연산 규칙

### Highlight 변경

Highlight는 현재 Transition bit를 보존하고 하위 7bit만 교체한다.

개념식:

~~~cpp
NewStencil = (CurrentStencil & TransitionBit) | HighlightValue;
~~~

`HighlightValue`는 `HighlightMask` 범위를 넘지 않아야 한다.

Highlight 제거는 `HighlightValue = 0`과 같다.

### Transition 시작

Transition은 현재 Highlight 값을 보존하고 bit 7만 켠다.

개념식:

~~~cpp
NewStencil = CurrentStencil | TransitionBit;
~~~

### Transition 종료

Transition bit만 제거하고 현재 Highlight 값을 그대로 남긴다.

개념식:

~~~cpp
NewStencil = CurrentStencil & HighlightMask;
~~~

### CustomDepthStencilWriteMask

Stencil 값의 bit 분할만으로 기존 렌더링 동작이 자동 보존되지는 않는다.

공용 연산은 최종 Stencil 상태와 함께 WriteMask도 다음 규칙으로 설정한다.

~~~text
Transition bit ON
-> ERSM_255

Transition bit OFF
-> ERSM_Default
~~~

즉 Transition 동안에는 기존 Transition 표현과 동일하게 전체 Stencil write를 허용하고, Transition이 끝나면 Highlight 기본 동작으로 복귀한다.

Highlight 값이 Transition 도중 변경되어도 Transition bit가 켜져 있다면 `ERSM_255`를 유지한다.

### RenderCustomDepth

최종 Stencil 값으로 활성 여부를 결정한다.

~~~text
NewStencil != 0
-> RenderCustomDepth = true

NewStencil == 0
-> RenderCustomDepth = false
~~~

따라서 다음이 모두 성립해야 한다.

~~~text
Highlight only
-> ON

Transition only
-> ON

Highlight + Transition
-> ON

둘 다 없음
-> OFF
~~~

## 5. UMAHighlightComponent 변경

Highlight의 실제 책임은 그대로 유지한다.

~~~text
AddTarget()
SetHighlight()
FRequest
Requester
Priority
Color Hue -> Highlight Stencil 값 변환
~~~

`ApplyHighlight()`는 Primitive의 Stencil / CustomDepth / WriteMask를 직접 제어하지 않는다.

목표 호출:

~~~text
ActiveRequest 있음
-> FMARenderStencil::SetHighlightValue(..., ActiveRequest->StencilValue)

ActiveRequest 없음
-> FMARenderStencil::SetHighlightValue(..., 0)
~~~

Transition bit는 직접 수정하지 않는다.

`ConvertColorHueToStencilValue()`의 현재 1~37 결과는 유지한다.

### 제거할 Highlight API / 상태

현재 `SetHighlightEnabled()`와 `bHighlightEnabled`의 사용처가 Transition Visibility뿐이라면 이번 직접 결합 제거와 함께 삭제한다.

~~~text
SetHighlightEnabled()
bHighlightEnabled
~~~

이 때문에 `MAHighlightComponent.h`도 이번 선커밋 범위에 포함한다.

Highlight suspend/resume 개념을 다른 이름으로 다시 만들지 않는다.

## 6. UMASpaceTransitionVisibilityComponent 변경

이 Component의 최종 책임은 다음 하나다.

> 등록된 Primitive의 Transition visibility bit를 켜고 끈다.

유지:

~~~cpp
void AddTarget(UPrimitiveComponent* Target);
~~~

Mask가 사용하는 내부 진입점도 유지한다.

~~~cpp
void SetVisibleThroughTransition(bool bVisible);
~~~

삭제:

~~~text
SetHighlighter()
Highlighter
FTargetState
SavedTargetStates
EnableTarget()
Highlight 직접 비활성화 / 재활성화
이전 RenderCustomDepth snapshot
이전 StencilValue snapshot
이전 WriteMask snapshot
~~~

최종 동작:

~~~text
SetVisibleThroughTransition(true)
-> 각 Target에 FMARenderStencil::SetTransitionVisible(Target, true)

SetVisibleThroughTransition(false)
-> 각 Target에 FMARenderStencil::SetTransitionVisible(Target, false)
~~~

Highlight 값을 직접 읽거나 수정하지 않는다.

## 7. MAPlayerCharacter 연결 정리

현재 Player 생성 코드의 직접 Highlight 연결을 제거한다.

~~~cpp
SpaceTransitionVisibilityComponent->SetHighlighter(GetHighlightComponent());
~~~

다음 Target 등록은 유지한다.

~~~cpp
SpaceTransitionVisibilityComponent->AddTarget(GetMesh());
SpaceTransitionVisibilityComponent->AddTarget(WeaponComponent);
SpaceTransitionVisibilityComponent->AddTarget(MountMesh);
~~~

결과적으로:

~~~text
UMAHighlightComponent
x
UMASpaceTransitionVisibilityComponent
~~~

두 Component 사이에 include, 멤버 참조, runtime enable/disable 호출이 없어야 한다.

## 8. Magic Circle 영향

Magic Circle은 Highlight Component 없이 Transition Visibility만으로 정상 동작해야 한다.

현재처럼 Circle Mesh만 Transition Visibility Target으로 등록한다.

~~~text
CircleMesh
-> Transition bit ON / OFF
~~~

Highlight Component를 추가하지 않는다.

`MAMagicCircle`의 Occupancy, Transition API, Transform, Sound 로직은 이번 범위에서 수정하지 않는다.

## 9. Mask Component 영향

`UMASpaceTransitionMaskComponent`는 이번 refactor의 상태 머신 / Presentation 로직 변경 대상이 아니다.

현재처럼 Transition Visibility에 다음만 요청한다.

~~~text
SetVisibleThroughTransition(true)
SetVisibleThroughTransition(false)
~~~

Mask는 Highlight bit 배치, Highlight 요청, Stencil 세부 연산을 알지 않는다.

## 10. Material 계약

코드가 bit를 분리해도 Material이 raw CustomStencil을 이전 방식으로 해석하면 기능이 깨진다.

따라서 실제 Material Graph까지 같은 계약으로 확인한다.

### Highlight Material

현재 `PP_Highlight`는 Highlight 값으로 하위 7bit만 사용해야 한다.

최종 의미:

~~~text
HighlightStencil = CustomStencil low 7bit
~~~

Material Graph에서는 다음과 동등한 결과를 만든다.

~~~text
CustomStencil % 128
~~~

Transition bit가 켜져 있어도 기존 Highlight 1~37 값이 그대로 해석되어야 한다.

### Transition Material

Transition Material은 최상위 bit만 Transition visible subject 의미로 사용한다.

최종 판정 의미:

~~~text
CustomStencil >= 128
-> Transition visible subject
~~~

하위 Highlight 값과 관계없이 bit 7이 켜져 있으면 Transition 예외 대상이어야 한다.

실제 Transition Material asset 이름은 구현 시 현재 참조를 따라 확인한다. 추측해서 새 Material을 만들지 않는다.

Material Graph 변경이 필요하면 실제 수정 asset을 이번 구현 / 검증 범위에 포함한다.

## 11. 금지할 구현

이번 범위에서 다음은 만들지 않는다.

~~~text
StencilManager
StencilRegistry
StencilAllocator UObject
RenderStateCoordinator Component
namespace MARenderStencil
Highlight를 상속한 TransitionVisibility
TransitionVisibility를 상속한 Highlight
Highlight suspend/resume 대체 API
현재 존재하지 않는 제3/제4 Stencil 사용자용 generic bit allocator
~~~

현재 실제로 충돌하는 두 기능의 계약만 닫는다.

## 12. 작동 검증

Build만으로 완료 판정하지 않는다.

### Case A - Highlight only

~~~text
Highlight ON
Transition OFF
-> 기존 Highlight 색상 정상
-> Stencil low 7bit에 현재 Highlight 값
-> WriteMask ERSM_Default
-> CustomDepth ON
~~~

### Case B - Transition only

~~~text
Highlight OFF
Transition ON
-> Stencil bit 7 ON
-> WriteMask ERSM_255
-> Player / Magic Circle이 Transition Mask 예외로 정상 표시
-> CustomDepth ON
~~~

### Case C - Highlight then Transition

~~~text
Highlight ON
-> Transition ON
-> 기존 Highlight low 7bit 보존
-> bit 7 ON
-> WriteMask ERSM_255
-> Transition 표시 정상
~~~

### Case D - Transition 중 Highlight 변경

~~~text
Transition ON
-> Highlight A
-> Highlight B
-> bit 7 유지
-> WriteMask ERSM_255 유지
-> Transition 종료 후 최신 Highlight B 유지
~~~

### Case E - Transition 중 Highlight 제거

~~~text
Transition ON
-> Highlight OFF
-> bit 7 유지
-> CustomDepth 계속 ON
-> Transition 종료 후 Stencil 0
-> WriteMask ERSM_Default
-> CustomDepth OFF
~~~

### Case F - Transition 종료 시 Highlight 유지

~~~text
Highlight ON + Transition ON
-> Transition OFF
-> Highlight low 7bit만 남음
-> WriteMask ERSM_Default
-> CustomDepth ON
~~~

### Case G - Magic Circle

~~~text
Highlight Component 없음
Transition Visibility만 사용
-> Close / Open에서 정상 표시
-> 종료 후 Stencil 0 / CustomDepth OFF
~~~

### Case H - Player targets

~~~text
Body
Weapon
Mount
-> 모두 같은 bit contract로 정상 표시
~~~

## 13. 코드 검증

필수:

~~~text
Project build 통과
git diff --check 통과
~~~

추가 확인:

~~~text
UMAHighlightComponent가 UMASpaceTransitionVisibilityComponent를 include / 참조하지 않음
UMASpaceTransitionVisibilityComponent가 UMAHighlightComponent를 include / 참조하지 않음
SetHighlighter 검색 결과 0
SetHighlightEnabled 검색 결과 0
bHighlightEnabled 검색 결과 0
SavedTargetStates 검색 결과 TransitionVisibility에서 0
TransitionVisibility의 Stencil snapshot 코드 0
FMARenderStencil 외에 동일 bit / WriteMask 연산 중복 없음
~~~

`MASpaceTransitionMaskComponent`, TransitionTypes, PlayerController RPC, TransitionSubsystem의 로직은 이 검증 범위에서 수정하지 않는다.

## 14. 선커밋 범위

작동 확인과 코드 검수를 모두 통과한 뒤 독립 선커밋으로 닫는다.

예상 코드 범위:

~~~text
Source/P_MA/Private/MARenderStencil.h
Source/P_MA/Private/Convenience/MAHighlightComponent.h
Source/P_MA/Private/Convenience/MAHighlightComponent.cpp
Source/P_MA/Private/Level/Transition/MASpaceTransitionVisibilityComponent.h
Source/P_MA/Private/Level/Transition/MASpaceTransitionVisibilityComponent.cpp
Source/P_MA/Private/Player/MAPlayerCharacter.cpp
~~~

Material Graph 변경이 필요하면 실제 수정된 Highlight / Transition Material asset도 같은 commit에 포함한다.

포함하지 않을 것:

~~~text
MASpaceTransitionMaskComponent 상태 머신 / Presentation 로직 변경
MAMagicCircle 로직 변경
MASpaceTransitionTypes
MAPlayerControllerBase Transition RPC
MASpaceTransitionSubsystem
Transition GameplayTag rename
Map / Config / GameMode
기타 미검수 Transition 변경
~~~

현재 working tree에 위 파일들의 다른 변경이 섞여 있다면 이번 Stencil ownership 관련 hunk만 부분 스테이징한다.

## 15. Acceptance Criteria

### Responsibility

- `FMARenderStencil`은 Primitive의 공유 Stencil / WriteMask / CustomDepth 적용만 닫는다.
- Highlight는 Request / Priority / Color와 하위 7bit 의미만 소유한다.
- Transition Visibility는 등록 Target과 bit 7의 on/off 의미만 소유한다.
- Highlight와 Transition Visibility는 서로 직접 참조하지 않는다.

### API

- `FMARenderStencil` 외부 API는 `SetHighlightValue()`와 `SetTransitionVisible()` 두 개다.
- `GetHighlightValue()` / `HasTransition()` 같은 미사용 조회 API가 없다.
- `SetHighlightEnabled()` / `bHighlightEnabled`가 남지 않는다.

### State correctness

- Highlight 변경이 Transition bit를 보존한다.
- Transition 변경이 Highlight 값을 보존한다.
- Transition 중 Highlight가 바뀌어도 종료 후 최신 값이 남는다.
- Transition ON에서는 `ERSM_255`다.
- Transition OFF에서는 `ERSM_Default`다.
- 최종 Stencil이 0이 아니면 CustomDepth가 켜진다.
- 최종 Stencil이 0이면 CustomDepth가 꺼진다.

### Minimality

- 이전 상태 snapshot을 보관하지 않는다.
- Highlight suspend/resume API를 만들지 않는다.
- Manager / Registry / Allocator를 만들지 않는다.
- 현재 존재하지 않는 Stencil 사용자를 위해 generic allocation 시스템을 만들지 않는다.

### Material

- `PP_Highlight`는 하위 7bit를 Highlight 값으로 해석한다.
- Transition Material은 bit 7을 Transition visible subject로 해석한다.
- Highlight와 Transition이 동시에 활성화되어도 두 표현이 서로의 Stencil 정보를 파괴하지 않는다.

### Validation

- Case A~H를 확인한다.
- Build와 `git diff --check`를 통과한다.

### Commit

- 위 검증 전에는 commit하지 않는다.
- 검수 후 Stencil ownership 관련 파일 / hunk만 선커밋한다.
- 아직 미검수인 Transition orchestration 변경은 포함하지 않는다.

## 16. 구현 후 읽혀야 하는 형태

~~~text
Highlight request changes
-> UMAHighlightComponent
-> FMARenderStencil::SetHighlightValue()
-> low 7bit 변경
-> transition bit 보존
-> final WriteMask / CustomDepth 적용

Transition closes
-> UMASpaceTransitionMaskComponent
-> UMASpaceTransitionVisibilityComponent
-> FMARenderStencil::SetTransitionVisible(true)
-> bit 7 ON
-> Highlight 값 보존
-> ERSM_255

Transition opens
-> FMARenderStencil::SetTransitionVisible(false)
-> bit 7 OFF
-> current Highlight 값 유지
-> ERSM_Default
-> final Stencil 값에 따라 CustomDepth 유지 / 해제
~~~

이번 작업의 목표는 Custom Stencil을 범용 시스템으로 만드는 것이 아니다.

현재 실제로 충돌하는 Highlight와 Transition Visibility가 서로의 내부 상태를 직접 제어하지 않고도 같은 Primitive에서 정확하게 공존하도록 현재 계약을 닫는 것이다.
