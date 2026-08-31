# 01. Lobby Hub Mockup

> Status: Mockup-ready design contract
> Branch: `codex/mockup-level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. 목적

기존의 Fortnite / Apex Legends 계열 3D 프론트엔드형 Lobby를, 플레이어가 직접 움직이고 기능 오브젝트와 상호작용하는 **조작 가능한 3D Hub**로 교체한다.

이번 목업의 목표는 완성된 아트가 아니라 다음을 검증하는 것이다.

- 기존 Lobby 기능을 실제 Hub 공간 안에서 사용할 수 있는지;
- 기능 진입점을 월드 오브젝트 상호작용으로 통일했을 때 UX가 자연스러운지;
- 마법진이 Hub 공간의 중심축으로 기능할 수 있는지;
- 플레이어 Spawn을 Ragdoll 투입 연출로 처리했을 때 재미와 가독성이 있는지;
- 이후 Seamless Teleport 목업으로 자연스럽게 이어질 수 있는지.

## 2. 전체 Lobby Rework 작업 구분

Lobby Rework는 크게 세 작업으로 나눈다.

1. **Lobby 변경 및 자연스러운 Hub 공간**
2. **새로운 Lobby 이동 공간 내부 구성**
3. **Seamless Teleport 및 마법진**

현재 문서는 **1번 Hub 목업**만 다룬다.

2번과 3번의 세부 전환/텔레포트 구현은 별도 문서에서 다룬다.

## 3. 핵심 방향

### Decision

기존 Lobby를 유지한 뒤 별도 Hub로 이동하지 않는다.

기존 Lobby 자체를 **조작 가능한 Hub로 교체**한다.

즉 기존의:

- 캐릭터 슬롯 전시;
- Ready 버튼;
- Start 버튼;
- Loadout 버튼;
- Invite 버튼;

중 공간 행동으로 대체할 수 있는 부분은 실제 Hub 상호작용으로 이동한다.

### 기본 원칙

> 주요 Lobby 기능의 진입점은 월드 오브젝트로 통일한다.

기능 내부는 기존 UI를 재사용해도 된다.

예:

- Loadout Station 상호작용 -> 기존 Loadout UI 진입;
- Party / Invite Object 상호작용 -> 기존 Invite / Party UI 진입;
- Magic Circle 진입 -> Ready 의미;

이번 목업은 기존 UI 내부 기능을 재설계하는 작업이 아니다.

## 4. Hub 공간의 큰 구조

Hub는 지나치게 큰 탐험 공간으로 만들지 않는다.

주요 기능 오브젝트까지 몇 초 안에 접근 가능한 작은 공간을 우선한다.

공간은 크게 세 영역으로 본다.

### 4.1 Front / Player Activity Area

플레이어가 실제로 사용하는 주요 생활 공간.

포함:

- Spawn / Arrival Area;
- 자유 이동 및 대기 공간;
- Loadout 진입 오브젝트;
- Party / Invite 진입 오브젝트;
- 향후 추가 가능한 Lobby 기능 오브젝트.

### 4.2 Magic Circle Axis

마법진은 별도 방 깊숙이 숨기지 않는다.

Hub의 시각적 중심축에 두되, 완전한 정중앙보다 **약간 뒤쪽**에 배치한다.

이유:

- 플레이어가 Hub 어디서든 마법진의 존재를 인지할 수 있어야 함;
- Ready / 출발 기능이 공간의 핵심임을 보여줘야 함;
- 이후 Seamless Teleport의 공간 기준점으로 사용하기 쉬워야 함;
- 마법진 뒤쪽에 별도 연출 공간을 확보할 수 있음.

### 4.3 Rear Presentation Area

마법진 뒤쪽은 초기 목업에서 기능 오브젝트로 채우지 않는다.

이 공간은 향후:

- 마법진을 관장하는 상위 존재;
- 거대한 제단 / 장치;
- 마법진 활성화 연출;
- 스토리 / 세계관 연출;

등을 위한 무대로 남긴다.

현재 목업에서는 단순 Geometry / Placeholder만 있어도 된다.

### 공간 의미

> 앞쪽 = 플레이어를 위한 Hub.
> 가운데/뒤쪽 경계 = 마법진.
> 뒤쪽 = 상위 존재와 마법진 연출을 위한 공간.

## 5. 기능 오브젝트

### 5.1 Loadout Object

Loadout 기능은 월드 오브젝트를 진입점으로 사용한다.

목업에서는 장비대 / 보관함 / 단순 Placeholder Actor 중 하나면 충분하다.

상호작용 흐름:

1. 플레이어가 Loadout Object에 접근;
2. 기존 Interaction 기능으로 상호작용;
3. 기존 Loadout UI / Preview 흐름 진입;
4. 종료 후 Hub 조작으로 복귀.

### Important

Head / Body / Weapon / Mount마다 별도 월드 오브젝트를 만들지 않는다.

초기 목업은 **하나의 Loadout 진입점**에서 기존 Loadout 전체를 사용한다.

### 5.2 Party / Invite Object

Party / Invite 기능 역시 월드 오브젝트를 단일 진입점으로 사용한다.

목업에서는 통신 장치 / 게시판 / 단순 Placeholder Actor면 충분하다.

상호작용 후 기존 Steam Invite 또는 현재 Lobby Invite 흐름을 호출한다.

Invite 접근성을 위해 별도 고정 UI 버튼을 유지하는 것을 초기 요구사항으로 두지 않는다.

## 6. Ready와 Magic Circle

### Decision

기존 Ready 버튼은 공간 행동으로 대체한다.

> 플레이어가 Magic Circle의 Ready 영역 안에 들어가 있는 상태 = Ready.

Ready 영역을 벗어나면 Ready가 해제되는 방식이 기본 후보다.

이번 Hub 목업에서는 다음만 검증한다.

- Magic Circle 위치가 Hub 기능 흐름에 방해되지 않는지;
- 여러 플레이어가 동시에 올라가 있을 공간이 충분한지;
- Ready 여부가 캐릭터 위치만으로도 직관적으로 읽히는지;
- 시각 효과로 Ready 상태를 추가 표시하기 쉬운지.

### Deferred

다음은 Seamless Teleport 문서에서 확정한다.

- 전원 Ready 시 자동 출발 여부;
- Host 최종 Start 승인 여부;
- Countdown;
- 전환 연출;
- 목적지 생성 완료 확인;
- 실제 Teleport 실행 시점.

## 7. Spawn / Arrival 연출

### 7.1 목표

플레이어가 Hub에 처음부터 서 있는 방식 대신, **Ragdoll 상태로 Hub 공간 안으로 투입되는 연출**을 사용한다.

기본 흐름:

1. 플레이어의 Hub 진입/초기화 시작;
2. Spawn 위치 계산;
3. 캐릭터를 Ragdoll 상태로 투입;
4. 중앙 바닥의 Landing Region을 향해 날아옴;
5. 바닥과 실제 물리 충돌;
6. 별도의 Landing Animation 없이 Ragdoll 상태로 널브러짐;
7. 필요한 로딩/초기화 완료;
8. Get Up;
9. 입력 활성화 및 HubActive 상태 진입.

착지 전용 상태나 착지 애니메이션을 두는 것이 목표가 아니다.

## 8. Spawn Volume

### Decision

고정 Player Spawn Slot 대신 **사각형 Spawn Volume**을 사용한다.

Spawn Volume에는 다음 기준이 있다.

- 사각형 평면 범위;
- 중심점 `C`;
- Hub의 대표 Camera / Layout 기준축;
- Corner Margin;
- 중앙 Landing Region.

### 8.1 Random Spawn Edge 선택

평면상 랜덤 방향을 선택하고, Spawn Volume 중심 `C`에서 해당 방향으로 Ray를 보낸다.

Ray와 사각형 외곽이 만나는 점을 Spawn 측 기준점으로 사용한다.

### 8.2 Camera 축 제외

전체 방향을 균등하게 허용하지 않는다.

Hub의 대표 Camera가 바라보는 방향과 **평행하거나 매우 가까운 한 축 주변**은 제외한다.

해당 축의 양 방향 모두 작은 금지 각도 범위를 둘 수 있다.

그 외 허용 범위에서는 방향을 완전 랜덤으로 선택해도 된다.

목적:

- Camera 시선축 정면/후면에서 반복적으로 튀어나오는 기계적인 연출 방지;
- 화면을 가리거나 카메라 방향으로 직접 날아오는 결과 감소;
- 다양한 측면/대각선 투입 연출 확보.

금지 각도의 정확한 수치는 목업 튜닝 값이다.

### 8.3 Corner 회피

Ray 교점이 사각형 모서리에 너무 가까운 경우 해당 방향을 버리고 다시 선택한다.

`CornerMargin`은 튜닝 값으로 둔다.

목적:

- 모서리 Geometry와 충돌하는 이상한 발사;
- 벽 두 면에 동시에 간섭하는 상황;
- 반복적으로 꺾인 구석에서 등장하는 부자연스러운 결과;

를 피한다.

## 9. Landing Target

### Decision

단순히 `SpawnPoint -> Center` 방향으로 고정 Impulse를 주는 방식은 사용하지 않는다.

이 방식은 Ragdoll의 물리 오차 때문에 중앙을 통과해 반대 벽까지 날아갈 가능성이 있다.

대신 **착지 목표 위치를 먼저 결정**한다.

### Landing Region

Spawn Volume 중앙 바닥에 작은 Landing Region을 둔다.

개념:

`LandingTarget = Center + RandomOffsetWithinLandingRegion`

모든 플레이어가 정확히 한 점에 떨어질 필요는 없다.

중앙 주변의 제한된 영역 안에서 서로 다른 Target을 선택한다.

### Launch

Spawn 측 위치에서 선택된 LandingTarget 근처에 떨어질 수 있도록 초기 속도 / 힘을 계산하거나 조절한다.

목표는:

- 외곽의 랜덤한 방향에서 등장;
- 중앙 바닥 근처에 떨어짐;
- 실제 Ragdoll 충돌 이후 조금 굴러가거나 자세가 달라지는 것은 허용;
- 정상 결과로 반대편 벽까지 날아가지는 않음.

정확한 탄도 계산 방식과 Impulse/Velocity API 선택은 구현 목업에서 결정한다.

## 10. Ragdoll 이후 Get Up

캐릭터는 바닥에 떨어진 뒤 별도의 착지 애니메이션 없이 Ragdoll 상태를 유지한다.

Get Up 조건의 핵심은 **필요한 초기 로딩/초기화가 완료되었는가**이다.

초기화가 완료되면:

1. Ragdoll 종료 준비;
2. 현재 자세에 맞는 Get Up 처리;
3. 정상 Character 상태 복귀;
4. 입력 활성화.

최소 누워있는 연출 시간 같은 값은 필요하면 목업 튜닝 값으로 추가할 수 있지만 현재 필수 계약은 아니다.

## 11. 네트워크 관점의 범위

이번 문서는 세션 생성/참가 네트워크 구조를 다시 설계하지 않는다.

Hub 목업에서 필요한 최소 요구만 둔다.

- 다른 플레이어가 새 Player의 투입/Ragdoll/Get Up을 볼 수 있어야 함;
- Ready 상태는 멀티플레이에서 일관되게 보여야 함;
- Loadout / Invite 진입은 기존 권한/네트워크 규칙을 가능한 한 재사용;
- 실제 Seamless Teleport 동기화는 3번 작업으로 미룸.

## 12. 목업 구현 우선순위

### Phase A - Hub Blockout

- 기존 Lobby Map 또는 별도 Mockup Map에서 Hub Blockout;
- Front Player Area;
- Magic Circle 위치;
- Rear Presentation Area;
- 기능 오브젝트 Placeholder 배치.

### Phase B - Interaction Entry Points

- Loadout Object -> 기존 Loadout 진입;
- Party / Invite Object -> 기존 Invite 진입;
- 기존 로비 버튼 의존성 제거 가능 여부 확인.

### Phase C - Magic Circle Ready

- Ready Collision / Volume;
- 진입 시 Ready;
- 이탈 시 Ready 해제;
- 멀티플레이 상태 표시.

### Phase D - Ragdoll Arrival

- Spawn Volume;
- Camera 축 제외 랜덤 Ray;
- Corner 회피;
- 중앙 Landing Region Target;
- Ragdoll 투입;
- 중앙 바닥 착지;
- 로딩 완료 후 Get Up.

## 13. 목업 완료 조건

다음을 만족하면 1번 Lobby Hub 목업은 성공으로 본다.

- 플레이어가 기존 메뉴형 Lobby가 아니라 실제 조작 가능한 Hub에서 시작함;
- Loadout과 Invite 기능을 월드 오브젝트 상호작용으로 진입 가능함;
- 주요 기능들이 너무 멀리 떨어져 있지 않아 기존 Lobby보다 과하게 불편하지 않음;
- Magic Circle이 Hub 중심축에서 명확한 랜드마크로 보임;
- Magic Circle 뒤쪽에 상위 존재/연출용 공간이 남아 있음;
- Magic Circle 진입/이탈로 Ready 상태를 표현 가능함;
- Spawn 방향이 Camera 기준 제외 축과 Corner 규칙을 지킴;
- Spawn 위치는 랜덤하지만 Landing은 중앙 Region 안쪽으로 충분히 통제됨;
- Ragdoll이 정상적인 경우 반대 벽까지 날아가는 결과가 반복되지 않음;
- 다른 플레이어가 새 Player의 등장 -> Ragdoll -> Get Up 흐름을 볼 수 있음;
- 로딩/초기화 완료 후 플레이어가 정상 Hub 조작 상태로 전환됨.

## 14. 이번 목업에서 하지 않는 것

다음은 현재 범위 밖이다.

- 최종 Lobby 아트;
- 상위 존재의 실제 디자인/애니메이션;
- 최종 Magic Circle VFX;
- 최종 Spawn VFX / 바닥 개방 연출;
- 전원 Ready 이후 실제 게임 시작 규칙 확정;
- Seamless Teleport;
- Generated Field 로딩 완료 Handshake;
- Late Join 중 필드 전환 처리;
- Hub의 장기 메타 기능 추가;
- 기능마다 별도 상호작용 시스템 신규 제작.

기존 기능을 최대한 재사용하면서 **공간 구조와 UX가 성립하는지 확인하는 것**이 이번 목업의 목적이다.

## 15. 목업에서 반드시 관찰할 것

구현 후 코드보다 플레이 감각을 우선 확인한다.

특히:

- 실제로 몇 초 걸어야 기능 오브젝트에 도달하는가;
- Player 여러 명이 중앙에 있을 때 공간이 좁거나 넓지 않은가;
- 마법진이 너무 공간을 차지하지 않는가;
- Rear Presentation Area가 너무 비어 보이지 않는가;
- Spawn 투입 연출이 재미있는가, 과하게 코믹한가;
- 랜덤 방향이 실제 화면에서 충분히 다양하게 보이는가;
- Ragdoll이 중앙 Landing Region에 자연스럽게 모이는가;
- 여러 명이 연속 Spawn될 때 서로 방해하는 정도가 허용 가능한가;
- Loadout / Invite를 오브젝트에서 여는 것이 실제로 귀찮지 않은가.

이 결과를 보고 공간 크기, 배치, Spawn 각도, CornerMargin, Landing Region 크기와 Launch 강도를 조정한다.
