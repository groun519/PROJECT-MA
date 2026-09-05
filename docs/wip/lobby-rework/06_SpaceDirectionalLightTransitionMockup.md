# 06. Space Directional Light Transition Mockup

## 목적

Streaming Space 전환 중 Destination의 Directional Light가 Source 화면에 미리 영향을 주는 문제를 해결한다.

현재 전환에서는 Destination Space가 Close 이전에 로드되어 같은 `UWorld`에 존재할 수 있다. 각 Space가 일반 `ADirectionalLight`를 그대로 가지고 있으면 두 조명이 동시에 월드에 영향을 주어, Mask가 닫히기 전 Source 화면의 밝기/색이 갑자기 변할 수 있다.

이 문제를 단순히 "Destination Light를 끈다"는 외부 절차로 흩뿌리지 않고, **Space의 Directional Lighting 자체를 하나의 객체로 모델링한다.**

핵심 목표는 다음과 같다.

- 각 맵은 에디터에서 실제 Directional Light를 보면서 그대로 조명 작업할 수 있다.
- Destination Space가 미리 로드되어도 현재 화면의 Directional Lighting은 변하지 않는다.
- Open 연출 동안 Source Lighting에서 Destination Lighting으로 자연스럽게 변화한다.
- 조명 속성 복사/보간/활성 전환의 세부사항은 외부 Transition 코드에 노출하지 않는다.
- 외부에는 "이 Space의 조명에서 저 Space의 조명으로 전환"하기 위한 작은 진입점만 제공한다.

---

## 핵심 객체

```text
AMASpaceDirectionalLight : ADirectionalLight
```

`AMASpaceDirectionalLight`는 단순히 `ADirectionalLight`에 프로젝트 이름을 붙인 클래스가 아니다.

하나의 객체가 다음 개념을 함께 가진다.

```text
한 Space의 Directional Lighting
= 에디터에서 직접 편집하는 실제 Directional Light
= 해당 Space가 원하는 authored lighting state
= 런타임에서 자신의 lighting participation을 관리하는 객체
= 다른 SpaceDirectionalLight로 전환하는 방법을 아는 객체
```

즉 외부가 `Intensity`, `LightColor`, `Temperature`, `Rotation` 등을 하나씩 읽고 직접 보간하는 구조로 만들지 않는다.

---

## 맵 구성

각 Streaming Space는 자신의 `AMASpaceDirectionalLight` 하나를 가진다.

```text
LobbyHubMap
├─ AMALevelRoot
└─ AMASpaceDirectionalLight
   └─ Lobby 조명 세팅을 에디터에서 직접 편집

MainMap1
├─ AMALevelRoot
└─ AMASpaceDirectionalLight
   └─ Battle/Desert 조명 세팅을 에디터에서 직접 편집
```

별도의 `FDirectionalLightSettings`, Lighting DataAsset, Runtime 전용 DirectionalLight를 기본 구조로 추가하지 않는다.

맵 제작자는 기존 Directional Light를 다루듯 해당 Actor를 회전시키고 Light Component 값을 수정하면서 결과를 즉시 확인할 수 있어야 한다.

`AMALevelRoot`는 필요하면 자신의 Space Lighting 객체를 참조/제공한다.

개념상:

```text
AMALevelRoot
└─ SpaceDirectionalLight -> AMASpaceDirectionalLight
```

정확한 프로퍼티/Getter 이름은 구현 시 현재 `AMALevelRoot` 스타일에 맞춘다.

---

## 런타임 불변 조건

### 1. 현재 Space의 Directional Light만 월드에 영향을 준다

Destination Space가 Streaming으로 미리 로드되어도 Destination `AMASpaceDirectionalLight`는 현재 화면에 영향을 주면 안 된다.

```text
Source Space Active
├─ Source Light       = runtime contribution ON
└─ Destination Light  = runtime contribution OFF
```

따라서 현재 발생하는 다음 현상을 없애는 것이 1차 목적이다.

```text
Source 화면 정상
-> Destination load/show
-> 아직 이동도 Close도 하지 않았는데 화면 밝기/색 변경
```

Destination이 언제 로드되든 Source Lighting은 Close 전까지 그대로 유지되어야 한다.

특히 Destination Light가 Streaming level이 나타나는 짧은 순간이라도 한 프레임 월드에 영향을 준 뒤 꺼지는 구조는 허용하지 않는다.

정확히 어떤 Unreal lifecycle 지점에서 runtime contribution을 차단할지는 구현 시 가장 단순하고 안전한 방법을 선택한다.

---

### 2. Lighting 변화는 Open 연출과 함께 진행한다

완전히 닫힌 순간 Destination 값으로 즉시 교체하는 것이 목표가 아니다.

원하는 연출은 다음과 같다.

```text
Close
-> Closed
-> Player Move
-> Open 시작
   Source Lighting
      ↓
      ↓ 자연스럽게 변화
      ↓
   Destination Lighting
-> Open 완료
```

즉 Mask Open의 진행과 함께 조명도 Source에서 Destination으로 변화한다.

이렇게 하면 Destination이 처음 보이는 시점부터 조명이 갑자기 한 번 바뀌는 것이 아니라, Space가 열리는 과정 자체에 Lighting transition이 포함된다.

---

## 객체지향 책임 경계

### `AMASpaceDirectionalLight`

자신의 조명 전환 방법을 스스로 안다.

내부 책임 후보:

- 자신의 authored Directional Light 상태 유지
- Destination Light의 authored 상태를 전환 목표로 사용
- 전환 가능한 값의 보간
- 보간하면 안 되는 값의 적절한 전환
- Source/Destination runtime contribution 관리
- 전환 완료 시 실제 활성 Lighting 주체를 Destination으로 넘김
- 전환 과정에서 authored Destination 값이 손상되지 않도록 관리

외부는 위 세부 구현을 알지 않는다.

### `UMASpaceTransitionSubsystem`

전체 Space Transition의 순서만 안다.

```text
Load
-> Close
-> Move
-> Open
-> Unload
```

Open 중 Lighting transition이 필요하다는 사실 정도만 연결하며, Directional Light의 개별 프로퍼티를 직접 조작하지 않는다.

다음과 같은 코드는 Subsystem 책임이 아니다.

```text
Get Intensity
Get LightColor
Get Temperature
Get Rotation
Lerp each property
Set Source visibility
Enable Destination component
...
```

이런 지식은 `AMASpaceDirectionalLight` 내부에 캡슐화한다.

### `AMALevelRoot`

자신의 Space가 가진 Directional Lighting 객체를 제공하는 역할까지만 한다.

Lighting 보간 규칙이나 Transition 상태를 소유하지 않는다.

---

## 외부 진입점

외부에 필요한 것은 "현재 Space Lighting을 Destination Space Lighting으로 전환"할 수 있는 작은 진입점이다.

개념 예시는 다음 정도다.

```cpp
SourceLight->TransitionTo(DestinationLight, Alpha);
```

또는 동일한 책임을 더 자연스럽게 표현하는 다른 API를 선택해도 된다.

중요한 것은 정확한 함수명이 아니라 다음 계약이다.

```text
호출자는
- Destination Light 객체
- 전환 진행 정도 또는 전환 시작 의도
정도만 전달한다.

AMASpaceDirectionalLight는
- 어떤 값을 어떻게 변화시키는지
- 어떤 Light가 실제로 영향을 주는지
- 완료 시 어떻게 ownership/activation을 넘기는지
를 내부에서 처리한다.
```

기존 Mask의 Open progress를 그대로 활용할 수 있다면 별도 Tick/Timeline/Transition State를 중복 생성하지 않는다.

반대로 단일 호출형 API가 현재 코드 흐름에 더 단순하다면 클래스 내부에서 진행을 소유할 수 있다.

**API 모양보다 외부에 조명 구현 세부사항이 새지 않는 것이 우선이다.**

---

## 전환 예시

### Destination load

```text
LobbyHubMap = Current Space
MainMap1    = Destination Space

Lobby Light
= active runtime lighting

MainMap1 Light
= authored settings는 존재
= runtime contribution 없음
```

MainMap1이 미리 Streaming되어도 Lobby 화면은 변하지 않는다.

### Close / Move

```text
Mask Close
-> 완전히 닫힘
-> Player를 Destination Circle 기준으로 이동
```

이 단계에서도 현재 Lighting 주체를 외부 코드가 직접 교체할 필요는 없다.

### Open

```text
Open Alpha 0.0
Lobby Lighting

Open Alpha 0.5
Lobby -> MainMap1 중간 Lighting

Open Alpha 1.0
MainMap1 Lighting
```

`AMASpaceDirectionalLight`가 이 변화를 캡슐화한다.

### Open 완료

최종적으로 Destination `AMASpaceDirectionalLight`가 해당 Space의 실제 활성 Lighting 객체가 된다.

Source는 이후 정상적인 Space Transition 흐름에 따라 Unload된다.

다음 전환에서는 동일한 구조가 반복된다.

```text
Lobby Light
-> Battle Light

Battle Light
-> Next Space Light

Next Space Light
-> Hub Light
```

특정 Hub/Battle 타입에 종속되지 않는다.

---

## 구현 시 중요한 점

### Authored state를 외부 데이터로 중복하지 않는다

`AMASpaceDirectionalLight`가 이미 Unreal Editor에서 편집 가능한 실제 Directional Light이므로, 같은 값을 별도 구조체나 DataAsset에 다시 작성하게 만들지 않는다.

필요한 내부 snapshot이 있다면 클래스 구현 세부사항으로만 둔다.

### 외부가 Lighting property 목록을 알지 않는다

새 Lighting 속성을 지원하게 되어도 `UMASpaceTransitionSubsystem`이 수정될 필요가 없는 구조가 목표다.

예를 들어 향후 다음 항목을 전환 대상에 추가하더라도:

```text
Intensity
Light Color
Temperature
Rotation
Indirect Lighting Intensity
Volumetric Scattering
기타 Directional Light 옵션
```

수정 범위는 가능한 한 `AMASpaceDirectionalLight` 안에 머물러야 한다.

### 모든 값을 반드시 Lerp할 필요는 없다

연속적으로 변화시키는 것이 자연스러운 값과 즉시 전환해야 하는 값은 다를 수 있다.

어떤 속성을 어떤 방식으로 처리할지는 `AMASpaceDirectionalLight`가 결정한다.

### 별도 범용 Environment 시스템으로 확장하지 않는다

현재 확인된 문제는 Directional Light다.

SkyLight, Fog, SkyAtmosphere, PostProcess 등에도 같은 문제가 실제로 확인되면 그때 각 객체의 성격에 맞는 해결을 검토한다.

이번 작업에서 `MASpaceEnvironmentManager`, 범용 Environment DataAsset, Environment Interface 같은 계층을 미리 만들지 않는다.

---

## 하지 않을 구조

다음 구조는 현재 기본안으로 사용하지 않는다.

### WorldRoot에 별도 Runtime DirectionalLight + Space별 Settings Data

```text
WorldRoot
└─ Runtime DirectionalLight

Space
└─ DirectionalLightSettings
```

코드에서는 단순할 수 있지만 맵 제작자가 실제 조명을 직접 보며 편집하기 어렵고, Unreal의 기존 Directional Light authoring 경험을 다시 데이터로 복제하게 된다.

### Subsystem이 LightComponent를 직접 조작

```text
TransitionSubsystem
-> Source Light 값 읽기
-> Destination Light 값 읽기
-> 직접 Lerp
-> 활성/비활성 직접 관리
```

이 경우 Space Directional Light가 데이터 보관 객체로 전락하고 Lighting transition 지식이 Subsystem에 누출된다.

### 여러 Space DirectionalLight를 동시에 활성화한 채 Intensity만 조절

Streaming 순간 두 Directional Light가 동시에 월드에 개입할 여지를 남기므로 현재 문제를 확실하게 제거하지 못한다.

---

## 완료 기준

- 각 Space에서 `AMASpaceDirectionalLight`를 일반 Directional Light처럼 직접 편집할 수 있다.
- Destination Space가 Close 이전에 로드되어도 Source Lighting이 변하지 않는다.
- 런타임에 현재 Space의 Directional Lighting만 실제 월드에 영향을 준다.
- Player Move 후 Mask Open 과정에서 Source Lighting이 Destination Lighting으로 자연스럽게 변화한다.
- Open 완료 시 Destination `AMASpaceDirectionalLight`가 다음 전환의 Source Lighting 객체가 된다.
- `UMASpaceTransitionSubsystem`은 Directional Light 개별 속성을 알지 않는다.
- Lighting transition 세부사항은 `AMASpaceDirectionalLight` 내부에 캡슐화된다.
- 별도 Settings 복제, Runtime DirectionalLight, Manager/DataAsset/범용 Environment Framework를 필요 없이 추가하지 않는다.
- Hub -> Battle뿐 아니라 모든 Space -> Space 전환에 동일한 구조를 재사용할 수 있다.
