# Transition Restricted Mockup

## 1. Purpose

Transition Restricted is a short-lived player restriction applied only after departure is committed and before the destination begins opening.

Its purpose is not to freeze the player or to create a general transition-state framework. It exists only to prevent the player from breaking the seamless transition contract while the source space is closing and the handoff is being prepared.

The restriction must stay narrow:

- block all skill use,
- cap movement speed,
- keep the player inside the active Transition Circle,
- allow normal walking and turning inside that circle,
- release the restriction as soon as destination Open begins,
- always release on Abort.

No interaction restriction is required.
No jump policy is required because the current game has no jump mechanic.

---

## 2. Timing Contract

### 2.1 Start

The restriction begins only after the Magic Circle ready condition has fully completed.

```text
All current players inside Magic Circle
-> maintain for 3 seconds
-> final recheck succeeds
-> departure is committed
-> Transition Restricted begins
```

During the 3-second ready countdown there is no Transition Restricted state.
A player may freely leave the circle during that period, which cancels and resets the ready countdown through the existing Magic Circle logic.

### 2.2 Active Window

```text
Departure committed
-> Loading
-> Closing
-> Handoff
-> Destination prepared for Open
```

Transition Restricted remains active through this whole interval.

### 2.3 End

The restriction ends when destination Open begins, not when Open finishes.

```text
Handoff complete
-> Destination Open begins
-> Transition Restricted ends immediately
-> player may leave Destination Circle
-> player movement speed returns to normal
-> skills become usable again
```

Open is presentation at this point. Once the player has already been handed off to the destination and Open starts, there is no gameplay reason to keep the player trapped inside the circle.

Abort is also an unconditional end condition.

```text
Any pre-Open Abort
-> Transition Restricted ends immediately
-> all temporary movement/skill restrictions are restored
```

---

## 3. Restricted Behavior

While active:

```text
All skills: blocked
Movement: allowed
Turning: allowed
Max movement speed: clamped to 350
Leaving active Transition Circle: blocked
External movement that would cross the circle boundary: constrained by the same boundary rule
Interaction: unchanged
```

The initial movement-speed cap is `350`.
This is a tuning value, not a permanent balance constant. Adjust it after in-game testing if the circle feels too restrictive or too loose.

The cap is an upper bound, not a forced movement speed.

```text
Actual move speed = min(CurrentCalculatedMoveSpeed, TransitionMaxMoveSpeed)
```

Examples:

```text
Current speed 250 -> 250
Current speed 340 -> 340
Current speed 600 -> 350
Current speed 3000 -> 350
```

This preserves slows and other reductions while preventing pre-existing or stacked movement-speed bonuses from bypassing the transition boundary.

---

## 4. Skill Blocking

Do not individually block dash, teleport, movement skills, attacks, or other ability categories.

Transition Restricted means all skills are unavailable.

Reason:

```text
Block Dash only
-> future movement skill may bypass restriction

Block Teleport only
-> speed-stacking skill may bypass restriction

Block all skills
-> future skills require no transition-specific maintenance
```

Prefer the existing project-wide ability-blocking policy/tag if it already expresses full skill-use blocking correctly.
Do not introduce a parallel generic transition-ability framework unless the existing policy cannot represent the requirement.

Transition code owns when the restriction starts and ends.
The ability system owns how skill use is actually blocked.

---

## 5. Movement Speed Clamp

Movement speed should continue to be calculated by the normal character movement/stat path.
Transition Restricted only adds the final upper limit.

Conceptually:

```text
CalculatedMoveSpeed = normal movement/stat calculation

if TransitionRestricted:
    FinalMoveSpeed = min(CalculatedMoveSpeed, 350)
else:
    FinalMoveSpeed = CalculatedMoveSpeed
```

Do not replace the current movement speed with a fixed value.
A fixed value could unintentionally make a slowed character faster during transition.

The clamp should be applied at the existing final movement-speed refresh point so changes to attributes, buffs, debuffs, or slow multipliers still pass through one authoritative calculation path.

---

## 6. Circle Boundary Constraint

### 6.1 Boundary Source

Do not create a second arbitrary transition radius if the active Magic Circle already owns the relevant radius.

Use the active Transition Circle as the source of boundary meaning.

The actual character-center movement radius should account for the character capsule so the capsule body does not visibly leave the circle.

Conceptually:

```text
AllowedCenterRadius
= CircleRadius
- CapsuleRadius
- Margin
```

`Margin` is a small implementation/tuning allowance for avoiding visible edge penetration or network jitter. Its exact value should be determined during implementation/testing rather than treated as a new gameplay setting unless necessary.

### 6.2 Primary Constraint

Do not use per-frame position clamping as the normal movement method.

Instead, when the character reaches the boundary, remove only the outward movement component.
Tangential movement should remain valid so the character naturally slides along the circle edge.

Conceptually:

```text
Desired movement / velocity
-> decompose relative to circle center
-> inward or tangential component: keep
-> outward component that would cross boundary: remove
```

Expected feel:

```text
player moves diagonally toward circle edge
-> outward component is rejected
-> sideways component remains
-> player slides along the invisible circular boundary
```

This behaves like an invisible circular wall without creating a physical collision ring.

### 6.3 Safety Correction

A positional clamp is allowed only as an exception-recovery mechanism.

If the character is already outside `AllowedCenterRadius` because of accumulated velocity, external impulse, network correction, precision error, or another unexpected movement path:

```text
if DistanceFromCircleCenter > AllowedCenterRadius:
    move character to nearest valid point inside/on AllowedCenterRadius
```

Normal movement should not depend on this correction every frame.

### 6.4 External Forced Movement

Do not build a separate list of every knockback/impulse source just for transition.

The same circle-boundary rule should prevent external movement from carrying the player out of the active circle.

If an external force points outward at the boundary, the outward component is removed.
If it still produces an invalid position, the safety correction restores the player to the nearest valid position.

If implementation proves that the current impulse system needs an explicit transition hook, add only the minimum owner-local hook required by the real code path.

---

## 7. Source to Destination Handoff

The boundary owner changes during handoff.

Before handoff:

```text
BoundaryCenter = Source Transition Circle
BoundaryRadius = Source Transition Circle radius
```

During player transfer:

```text
Source-relative transform
-> move player to Destination-relative transform
-> active boundary reference changes to Destination Transition Circle
```

After transfer but before Open begins:

```text
BoundaryCenter = Destination Transition Circle
BoundaryRadius = Destination Transition Circle radius
```

This switch must occur as part of the handoff so the player is never constrained against the old Source Circle after being moved to the Destination Space.

No separate full-movement-lock phase is required for handoff unless implementation testing reveals an actual race/problem that the existing restriction cannot prevent.

---

## 8. Ownership

### Magic Circle

Owns:

- circle location/transform,
- circle radius/boundary meaning,
- existing all-player ready condition and 3-second maintenance.

Does not own:

- transition restriction policy,
- skill blocking,
- movement-speed clamp,
- transition lifecycle.

### Space Transition Subsystem

Owns:

- when Transition Restricted starts,
- which Source/Destination Circle is currently active for the restriction,
- handoff-time boundary reference switch,
- when Transition Restricted ends,
- unconditional cleanup on Abort.

It should issue semantic restriction start/end intent rather than directly implementing all character/ability movement internals.

### Player / Character Feature Owners

Own their own mechanics:

- ability system applies/removes full skill-use blocking,
- movement path applies/removes the speed cap,
- movement path enforces the active circular constraint.

The transition subsystem must not duplicate each feature's internal implementation.

---

## 9. Abort and Cleanup Contract

Every exit path before Destination Open must restore the player restriction state.

Required cleanup:

```text
Transition Restricted active
-> Abort
-> skill block removed
-> speed cap removed
-> circle constraint removed
-> active transition-circle restriction reference cleared
```

Cleanup must be safe if called repeatedly or after partial setup.
Do not leave a player permanently ability-blocked or movement-constrained because one transition stage failed.

Destination Open start performs the same restriction release for the successful path.

---

## 10. Implementation Constraints

- Reuse existing ability-blocking policy where appropriate.
- Reuse the existing final movement-speed calculation path for the clamp.
- Do not add a physical collision ring around the Magic Circle unless the movement constraint proves insufficient in real testing.
- Do not create a custom CharacterMovement framework solely for this feature unless the current movement path genuinely cannot enforce the constraint cleanly.
- Do not add interaction restrictions.
- Do not add jump-specific logic.
- Do not add a generic restriction framework for hypothetical future transition rules.
- Keep the implementation traceable by feature ownership: Transition decides lifecycle, player-side owners execute their own restriction mechanics.

---

## 11. Acceptance Cases

### Case A - Normal departure

```text
All players remain in circle for 3 seconds
-> departure commits
-> skills blocked
-> speed capped at 350
-> players cannot leave Source Circle
-> Close completes
-> handoff to Destination
-> boundary switches to Destination Circle
-> Destination Open begins
-> all restrictions released immediately
```

### Case B - Player tries to run out after departure commit

```text
player reaches circle boundary
-> outward movement removed
-> tangential movement remains
-> player stays inside circle
```

### Case C - Extremely high pre-existing movement speed

```text
CurrentCalculatedMoveSpeed = 3000
-> Transition Restricted begins
-> FinalMoveSpeed = 350
```

### Case D - Existing slow

```text
CurrentCalculatedMoveSpeed = 220
-> Transition Restricted begins
-> FinalMoveSpeed remains 220
```

### Case E - External impulse toward outside

```text
impulse pushes toward boundary
-> outward movement component constrained
-> if position still exceeds radius, safety correction restores valid position
```

### Case F - Abort during Loading/Closing

```text
Transition Restricted active
-> transition aborts
-> skill block removed
-> movement cap removed
-> circle constraint removed
-> player resumes normal state in Source Space
```

### Case G - Handoff boundary switch

```text
player is constrained to Source Circle
-> handoff moves player to Destination
-> active restriction reference becomes Destination Circle
-> no correction toward old Source position occurs
```

### Case H - Open starts

```text
Destination Open begins
-> restriction ends immediately
-> player may leave circle while Open presentation continues
```

---

## 12. Codex Implementation Scope

Implement only Transition Restricted closure described by this document.

Do not include the separate transition participant disconnect policy, player-count scaling, monster scaling, BattleReady integration, or procedural battle generation work in this change.

Before implementation, inspect the current `feature/psw/level-system-rework` code and reuse existing ability/movement APIs where they already satisfy the contract.

Stop after:

1. restriction lifecycle is connected to departure commit / handoff / Open start / Abort,
2. all skill use is blocked while restricted,
3. movement-speed upper clamp works with normal attribute refreshes,
4. circular movement constraint and safety correction work,
5. Source -> Destination boundary switch works,
6. successful and Abort cleanup restore the player correctly,
7. Standalone and Listen Server/Client behavior for these cases is verified.
