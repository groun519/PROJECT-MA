# WIP Skill Module Cleanup Review

이 문서는 현재 작업 범위의 구현 및 검수 상태를 추적하는 임시 체크리스트다.
범위를 닫고 커밋한 뒤 제거한다.

## Status

- `완료`: 구현과 현재 범위의 검수를 마침
- `검수 필요`: 구현은 되어 있으나 코드, 데이터, 에셋 또는 PIE 검수가 남음
- `재검토`: Module/Sub 정책 변경으로 이전 판정을 다시 검수해야 함
- `미착수`: 아직 변경하지 않음
- `결정 필요`: 구현 전에 정책을 확정해야 함

## Scope Rules

- 속성 룬 JSON 정리가 끝나기 전에는 관련 생성 에셋을 빌드하지 않는다.
- 사용자가 옮긴 모듈 파일은 별도 항목으로 추적하고 임의로 되돌리지 않는다.
- 현재 범위가 모두 닫히기 전에는 커밋하지 않는다.

## Confirmed Decisions

- SubModule 저장 및 조립 구조에는 개수 제한을 두지 않는다.
- 플레이어의 인챈트 부여 개수는 밸런스 값으로 조절하며 기본값은 모듈당 3개로 둔다.
- 인챈트 부여 진입점과 UI는 동일한 제한값을 사용한다.
- 몬스터 와인드업 같은 내부 조립은 플레이어 인챈트 제한을 적용하지 않는다.
- 대상 부모를 지정하는 것 자체가 설계의 핵심인 기능만 Sub로 사용한다.
- 단독 작동 여부나 시퀀스 유무만으로 Module/Sub를 판정하지 않는다.
- 전역 반응으로 의미 있는 조합을 만드는 기능은 Module을 우선 검토한다.

## Review Matrix

| 범위 | 구현 | 코드/데이터 검수 | 에셋 빌드 | PIE |
| --- | --- | --- | --- | --- |
| Composure 15 Module 복귀 및 밸런스 조정 | 완료 | 완료 | 대기 | 대기 |
| Expansion 21 Sub 전환 및 밸런스 조정 | 재검토 | 재검토 | 대기 | 대기 |
| Fast Strike 22 Sub 전환 및 밸런스 조정 | 재검토 | 재검토 | 대기 | 대기 |
| Heavy Strike 24 Sub 전환 및 희귀도 조정 | 완료 | 완료 | 대기 | 대기 |
| 쿨다운 Sub 29, 30 | 완료 | 완료 | 대기 | 대기 |
| Withdrawn 35 제거 | 완료 | 완료 | 제거 완료 | 해당 없음 |
| 복합 모듈 제거: 16, 18, 27, 28 | 완료 | 완료 | 완료 | 필요 시 확인 |
| `PatchDamagePayload` 피해 타입 필터 및 적용 방식 패치 | 완료 | 완료 | 해당 없음 | 속성 룬과 함께 확인 |
| Fire Rune 36 개편 | 완료 | 완료 | 대기 | 대기 |
| Ice Rune 37 개편 및 Sub 이동 | 완료 | 완료 | 대기 | 대기 |
| Light Rune 38 보정 | 완료 | 완료 | 대기 | 대기 |
| Wind Rune 39 DoT 전환 | 완료 | 완료 | 대기 | 대기 |
| Charge 45 기능 복구 및 밸런스 정리 | 완료 | 완료 | 대기 | 대기 |
| Cold Blood 12 / Hot Blood 14 온도 빌드 개편 | 완료 | 완료 | 대기 | 대기 |
| Crash 17 Module 복귀 | 완료 | 완료 | 대기 | 대기 |
| Dash 19 Module 복귀 | 완료 | 완료 | 대기 | 대기 |
| Overfocus 58 추가 | 완료 | 완료 | 대기 | 대기 |
| 중복 Fire Elemental 52 제거 | 완료 | 완료 | 제거 완료 | 해당 없음 |

## Full Module Review Queue

`[x]`는 모듈의 역할, Module/Sub 타입, 데이터 구조, 밸런스, 스킬/모듈 쿨다운 검수를 마쳤다는 뜻이다.
에셋 빌드와 PIE 상태는 위의 Review Matrix에서 별도로 추적한다.

### Module

- [x] 1 - Ready Blue (미사용 중복 경고 모듈 제거)
- [x] 2 - Danger Warning (빨간 오버레이 + Blade VFX 1초, 추가 쿨다운 없음)
- [x] 3 - GoblinLordSweep (Goblin Lord 폐기로 제거)
- [x] 4 - DA_GoblinDash (기본 대시 패턴 유지)
- [x] 5 - DA_GoblinDashFast (고속 대시 패턴 유지)
- [x] 6 - Ground Slam (기본 10 + 공격력 50%, 에어본 1초 유지, FastGroundSlam의 죽은 M_16 참조 제거)
- [x] 7 - Middle Golem Dash (속도 350, 1초 돌진 유지)
- [x] 8 - Golem Attack (기본 10 + 공격력 50%, 경직 0.2초, 둔화 50% 2초 유지)
- [x] 9 - Golem Attack Ready (중형 골렘 공격 3종의 준비 몽타주 유지)
- [x] 10 - Small Golem Dash (속도 1000, 0.4초 돌진과 Crash/Knockback 조합 유지)
- [x] 11 - Absolute Zero (추가 쿨다운 없음)
- [x] 12 - Cold Blood (추가 쿨다운 없음)
- [x] 13 - Devour (스킬 쿨다운 +9초, 모듈 최저 쿨다운 3초)
- [x] 14 - Hot Blood (추가 쿨다운 없음)
- [x] 15 - Composure (스킬 쿨다운 +1.5초)
- [x] 17 - Crash (스킬 쿨다운 +7.5초)
- [x] 19 - Dash (스킬 쿨다운 +2.4초)
- [x] 20 - Death (스킬 쿨다운 제거)
- [x] 25 - Hurt (스킬 쿨다운 +3.4초, 모듈 쿨다운 1초)
- [x] 26 - Knockback (스킬 쿨다운 +1.25초, 모듈 쿨다운 0.2초)
- [x] 30 - Tempo (스킬 전체 쿨다운 20% 감소)
- [x] 31 - Inhibited (스킬 쿨다운 -4초, 사용 시 자신 50% 둔화 2.5초)
- [x] 33 - Pacifist (기본 스킬 쿨다운 제거, 최종 피해량 0)
- [x] 34 - Pain (스킬 쿨다운 -4초, 사용 시 현재 체력 5% 피해)
- [x] 40 - Barrier (Global 패시브, 스킬 쿨다운 검수 제외)
- [x] 41 - Hematophagy (Global 패시브, 스킬 쿨다운 검수 제외)
- [x] 42 - Sword (Global 패시브, 스킬 쿨다운 검수 제외)
- [x] 43 - Berserker Fury (복합 책임 모듈 제거)
- [x] 44 - Burst (스킬 쿨다운 +5초, 모듈 쿨다운 1초)
- [x] 46 - Projection (낮은 피해로 원거리 이점을 지불, 추가 쿨다운 없음)
- [x] 47 - Slam (기본 피해 10 + 공격력 60%, 경직 가치로 스킬 쿨다운 +0.3초)
- [x] 48 - Slash (기본 피해 9 + 공격력 50%, 추가 쿨다운 없음)
- [x] 49 - Sting (기본 피해 9 + 공격력 45%, 추가 쿨다운 없음)
- [x] 50 - Sweep (기본 피해 8 + 공격력 50%, 추가 쿨다운 없음)
- [x] 51 - Throwing (기본 피해 7 + 공격력 40%, 25% 둔화로 정정, 추가 쿨다운 없음)
- [x] 58 - Overfocus (추가 쿨다운 없음)

### Sub

- [x] 21 - Expansion (부모 모듈 범위 +15%, 스킬 쿨다운 +1.25초)
- [x] 22 - Fast Strike (부모 모듈 시전 속도 +20%, 스킬 쿨다운 +1.25초)
- [x] 23 - Gamble (부모 모듈 피해 편차 ±30%, 추가 쿨다운 없음)
- [x] 24 - Heavy Strike (부모 모듈 최종 피해 +25%, 스킬 쿨다운 +1.5초)
- [x] 29 - Rewind (스킬 쿨다운 고정 2초 감소)
- [x] 36 - Fire Rune (일반 피해를 화염 피해로 전환, 스킬 쿨다운 +2초)
- [x] 37 - Ice Rune (일반 피해를 얼음 피해로 전환, 스킬 쿨다운 +2초)
- [x] 38 - Light Rune (일반 피해의 50%를 치유로 전환, 스킬 쿨다운 +2초)
- [x] 39 - Wind Rune (1초/5회 DoT, 시전 속도 16%, 이동 속도 20% × 1초, 스킬 쿨다운 +2초)
- [x] 45 - Charge (최대 1.5초 차지, 최종 피해 최대 +150%, 스킬 쿨다운 +3초)

### Item

- [ ] 53 - HealPotion

### Removed

- [x] 1 - Ready Blue
- [x] 3 - GoblinLordSweep
- [x] 16 - Compression
- [x] 18 - Crushing Strike
- [x] 27 - Time Compression
- [x] 28 - UnderDog
- [x] 32 - MindDrift
- [x] 35 - Withdrawn
- [x] 52 - Fire Elemental

## Confirmed Work

- 16, 18, 27, 28의 소스 JSON과 생성 에셋을 제거했다.
- `MainShop`에서 16, 18, 27, 28 참조를 제거했다.
- `PatchDamagePayload`에 다음 범용 기능을 추가했다.
  - 현재 `DamageTypeTag` 기준 필터
  - `ApplicationMode` 변경
  - DoT 설정 변경
- Wind Rune 39는 일반 피해를 1초 동안 5회 DoT로 나누고, 시전 속도 16%와 1초 동안 이동 속도 20%를 제공하도록 정리했다.
- Fire Rune 36은 일반 피해 타입만 Fire로 변경하고 스킬 쿨다운을 2초 늘리도록 정리했다.
- Ice Rune 37은 Sub로 옮기고, 일반 피해 타입만 Ice로 변경하며 스킬 쿨다운을 2초 늘리도록 정리했다.
- Light Rune 38은 일반 피해의 50%만 Heal로 변경하고 스킬 쿨다운을 2초 늘리도록 정리했다.
- 중복 Fire Elemental 52의 소스 JSON과 생성 에셋을 제거했다.
- Composure 15는 일반 Module로 복귀하고 스킬 최종 피해 10%와 일반 피해 집중력 25%를 함께 제공하며, 스킬 쿨다운 +1.5초를 적용한다.
- Expansion 21은 범위 증가를 15%로 조정하고 추가 쿨다운 1.25초를 유지했다.
- Fast Strike 22는 시전 속도 증가를 20%로 조정하고 추가 쿨다운 1.5초를 유지했다.
- Heavy Strike 24는 부모 모듈의 최종 피해를 25% 증가시키고 스킬 쿨다운을 1.5초 증가시키며, 희귀도는 Rarity3를 사용한다.
- Charge 45는 최대 차지 시간을 1.5초로 맞추고 차지 비율에 따라 최종 피해 배율이 1.0~2.5가 되도록 연결했다.
- 쿨다운 계산 순서를 `기본 쿨다운 × 비율 배율 + 고정 증감값`으로 확장했다.
- 기존 SubModule의 추가 쿨다운은 고정 증감값으로 이전했다.
- Rewind 29는 별도 페널티 없이 쿨다운을 2초 고정 감소하도록 개편했다.
- Tempo 30은 일반 Module로 전환하고, 별도 페널티 없이 조립된 스킬 전체 쿨다운을 20% 감소하도록 개편했다.
- 비율 쿨다운 변경은 최종 스킬 쿨다운에 적용하는 일반 Module에서만 사용한다.
- 역할이 쿨다운 SubModule과 중복되는 MindDrift 32의 소스 JSON과 생성 에셋을 제거했다.
- `MainShop`과 `StartShop`에서 MindDrift 32 참조를 제거했다.
- 역할이 Rewind/Tempo와 중복되고 비범위 스킬에서 페널티를 회피할 수 있는 Withdrawn 35의 소스 JSON과 생성 에셋을 제거했다.
- `MainShop`과 `StartShop`에서 Withdrawn 35 참조를 제거했다.
- GameplayEffectAddon에 non-snapshot Attribute Based magnitude를 추가했다.
- Cold Blood 12는 저온 페널티 면역, 온도 반비례 이동/공격 속도, 추가 얼음 피해로 개편했다.
- Hot Blood 14는 고온 페널티 면역, 온도 비례 공격력, 추가 화염 피해로 개편했다.
- Crash 17은 일반 Module로 복귀하고 스킬 내 모든 기동마다 충돌 피해를 적용하도록 정리했다.
- Crash 17의 공격력 200%와 고정 피해 5를 새 피해 단가로 환산해 스킬 쿨다운을 +7.5초로 조정했다.
- Dash 19는 일반 Module로 복귀하고 각 시퀀스 시작마다 대시하도록 정리했다.
- Dash 19는 대시 1m당 1 CV 기준으로 2.4m 이동에 스킬 쿨다운 +2.4초를 적용한다.
- Overfocus 58은 100%를 초과한 집중력의 50%를 스킬의 추가 치명타 피해로 전환하도록 추가했다.
- Knockback 26은 모든 적중에 2.5m 넉백을 추가하고, 스킬 쿨다운 +1.25초와 적중 후 모듈 쿨다운 0.2초를 사용한다.
- Hurt 25는 0.9초 동안 공격력 40%의 DoT와 2초 동안 30% 둔화를 적용하고, 스킬 쿨다운 +3.4초와 모듈 쿨다운 1초를 사용한다.
- 시퀀스 Module 13, 47, 48, 49, 50, 51의 공통 스킬 시행 속도 15% 보정을 제거했다.
- Devour 13은 전투 가치에 따른 스킬 쿨다운 +9초와 영구 성장 및 쿨다운 감소 빌드의 반복 사용을 제한하는 모듈 최저 쿨다운 3초를 사용한다.
- 전체 51개 JSON의 파싱과 ModuleId 중복 검사를 통과했다.
- C++ 빌드, JSON RoundTrip/Validation 자동화, `git diff --check`를 통과했다.

## Pending Review

- Charge 45의 `Module.Exclusive.Unique`는 SubModule 장착 단계의 공통 독점 규칙이 구현될 때 함께 검증한다.

## Elemental Rune Target

- Fire 36: 기존 일반 피해의 타입을 Fire로 변경한다.
- Ice 37: 기존 일반 피해의 타입을 Ice로 변경한다.
- Light 38: 기존 일반 피해를 Heal로 변경하고 아군 대상 및 관련 비주얼을 적용한다.
- Wind 39: 피해 타입은 유지하고 기존 일반 피해를 1초, 5회 DoT로 변경한다.
- 네 룬은 모두 `Sub`이며 `Module.Exclusive.Elemental`을 유지한다.

## Next Steps

1. 사용자가 에디터에서 36, 37, 38, 39를 빌드한다.
2. Fire, Ice, Light, Wind의 PIE 동작을 확인한다.
3. 사용자가 에디터에서 29, 30을 빌드하고 고정 및 비율 쿨다운 감소를 PIE에서 확인한다.

## Deferred

- `Module.Exclusive.Elemental`의 Sub 장착 충돌 강제는 현재 범위에서 구현하지 않는다.
- Fire/Ice로 변환된 피해의 `Event.Skill.Hit` 발행 정책은 별도 피해 파이프라인 범위로 남긴다.
