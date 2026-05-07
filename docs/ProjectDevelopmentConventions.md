# Project Development Conventions

This document records project-specific development preferences that should guide future implementation and refactoring.

## General Structure

- Prefer direct, necessary abstractions over thin wrappers.
- Remove wrappers that only forward calls without adding meaning, validation, or ownership.
- Prefer explicit ownership and responsibility boundaries over broad utility dumping grounds.
- Keep tags pure. Do not encode runtime identity into gameplay tags.
- Prefer receiving-side disambiguation when event senders must remain semantically pure.
- Avoid namespace-based helper bags for gameplay systems. Prefer focused static classes when a stateless helper type is needed.
- Remove legacy code once the replacement path is stable, unless the old code still carries active content data.

## UMG Widgets

- Prefer required `BindWidget` over `BindWidgetOptional`.
- Use `BindWidgetOptional` only for decorative, debug-only, or genuinely optional UI elements.
- Core functional widgets should fail loudly if missing from a Widget Blueprint.
- Buttons, panels, primary images, and labels required for behavior should be required bindings.
- The goal is to catch missing widget bindings in the editor or compile flow instead of silently degrading at runtime.

Examples of required bindings:

- `ExpandButton`
- `RemoveButton`
- `SlotRowsPanel`
- `ModuleSocketsPanel`
- `ModuleEntriesPanel`
- `SlotIconImage`
- `ModuleIconImage`
- `InputText`
- `SlotNameText`
- `ModuleNameText`

Examples of optional bindings:

- Decorative borders
- Hover overlays
- Debug-only labels
- Optional badge containers

## Skill System

- The skill manager owns slot stacks and mutation requests.
- UI should request changes through `UMASkillManagerComponent`; it should not mutate arrays directly.
- Skill definitions are modular source data. Runtime assembled definitions are results, not authoring data.
- Event bindings may be local or global. Local bindings compare runtime scope; global bindings ignore scope.
- Gameplay event payloads should use engine-supported payload fields intentionally, not ad-hoc temporary fields.

## Damage And Effects

- Damage config should remain data-focused.
- Damage resolution and damage application should stay separated.
- Gameplay cues that are target hit feedback belong in damage hit config, not animation notify trace data.
- DoT damage should re-evaluate per tick rather than pre-roll one fixed final damage.
## Git Workflow

- The user owns commits. Codex should not run `git commit` unless the user gives an explicit direct command to commit.
- When a change set is ready, Codex should report verification results and suggest a staging/commit scope instead of committing automatically.

## Blueprint Usage

- Prefer Blueprints for asset assignment, layout, visual composition, and data entry.
- Do not rely on Blueprint graph node wiring for core gameplay or UI behavior in new systems.
- C++ should own behavior flow, mutation requests, and runtime state changes.
- Expose functions to Blueprint only when a Blueprint graph is intentionally part of the design.
- Widget Blueprints should usually bind required widgets and assign classes/materials/assets, not drive logic through event graphs.

## C++ Style

- Prefer removing one-line forwarding wrappers unless the wrapper names a meaningful domain concept.
- Prefer inline definitions in headers for trivial one-line getters.
- Keep runtime behavior APIs in C++ unless Blueprint graph usage is explicitly intended.
- Blueprint asset fields are still valid for data entry, widget binding, class assignment, material assignment, and editor-authored defaults.
