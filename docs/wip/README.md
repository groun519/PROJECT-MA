# WIP Design Mockups

> Branch: `codex/mockups`  
> Storage model: orphan branch / design-only workspace  
> Implementation reference: current `develop` or the actual feature branch named by each topic  
> Purpose: PROJECT-MA의 구현 전 설계/목업 문서를 한 브랜치에서 기능별 파일로 관리한다.

## 1. Branch policy

이 브랜치는 기능별 목업 브랜치를 계속 만드는 대신 모든 WIP 설계를 한 곳에서 관리한다.

원칙:

1. 새로운 목업 주제마다 별도 Git branch를 만들지 않는다.
2. 주제별로 `docs/wip/<topic>/` 폴더를 추가한다.
3. 이 브랜치는 orphan 기반이며 실제 프로젝트 코드의 스냅샷을 보관하지 않는다.
4. 실제 Source/Content/Data/Config/Plugins 구현 파일을 이 브랜치에 추가하지 않는다.
5. 코드 검증이 필요하면 이 브랜치의 파일을 기준으로 삼지 않고 현재 `develop` 또는 해당 feature branch를 직접 읽는다.
6. 구현이 시작되면 각 문서가 가리키는 실제 feature branch에서 작업한다.
7. 목업 브랜치는 설계 문서의 공유 작업공간이며 구현 브랜치의 대체물이 아니다.
8. 확정 설계와 실험/보류 항목을 문서 안에서 명시적으로 구분한다.
9. 목업 브랜치 전체를 구현 브랜치에 merge하지 않는다. 확정된 결정만 실제 문서/코드로 의도적으로 이관한다.

## 2. Current workspaces

### `level-system-rework/`

절차적 맵 생성, Area/Connection/Path/Floor/Environment와 Monster NavMesh/고지대 이동을 포함한 Level System Rework 설계.

### `lobby-rework/`

Lobby Hub, Seamless Transition, Runtime Generation Rebuild 관련 목업.

### `monster-system-rework/`

Shared Trait Tree, Monster variation, Individual Behavior, StateTree/BT/EQS/Skill System 역할 분리와 구현 계획.

## 3. Folder rule

권장 구조:

```
docs/
└ wip/
  ├ README.md
  ├ level-system-rework/
  ├ lobby-rework/
  ├ monster-system-rework/
  └ <next-topic>/
```

하나의 큰 문서에 모든 내용을 넣기보다 기능 단위로 파일을 분리한다.

예:

```
<topic>/
├ README.md
├ 01_Context.md
├ 02_CoreDesign.md
├ 03_Architecture.md
└ 04_ImplementationPlan.md
```

문서 개수와 이름은 주제 복잡도에 맞게 조정한다.

## 4. Status labels

각 WIP 문서는 필요에 따라 다음 상태를 사용한다.

- **Fact**: 코드/엔진/프로젝트에서 확인된 사실
- **Requirement**: 반드시 만족해야 하는 요구
- **Decision**: 현재 채택된 설계
- **Candidate**: 유력하지만 아직 검증 중인 안
- **Experiment**: 실제 구현/플레이 테스트로 판단할 내용
- **Deferred**: 현재 범위에서 구현하지 않는 내용
- **Open Question**: 의도적으로 미결정 상태인 내용

## 5. Implementation handoff

목업이 구현 가능한 수준까지 정리되면 해당 주제 문서에 다음을 남긴다.

- 구현 순서
- 기존 코드에서 재사용할 책임/타입
- 추가될 새 책임
- 제거할 레거시
- 검증/디버그 방법
- 성공 조건
- 보류 범위

Codex는 문서의 `Decision`을 임의로 변경하지 않고, 실제 코드와 충돌하는 경우 차이를 먼저 보고한다.
