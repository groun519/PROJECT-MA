# Project Development Conventions

This document records project-specific development preferences that should guide future implementation and refactoring.

## General Structure

- Prefer direct, necessary abstractions over thin wrappers.
- Remove wrappers that only forward calls without adding meaning, validation, or ownership.
- Prefer explicit ownership and responsibility boundaries over broad utility dumping grounds.
- Prefer intent-revealing call sites. A reader should understand the operation from the call itself, e.g. `SetHighlighted(MeshComponent, bActive)`.
- Avoid registration/cache-style APIs when the caller can pass the actual target directly. Do not add `AddTarget()`-style setup unless persistent ownership or reuse genuinely needs it.
- Do not add future-proof product types, branches, wrappers, or data fields before the corresponding feature is implemented end-to-end.
- Abstractions are welcome when they make the usage site read like the domain action and hide C++ syntax noise. For example, an interaction setup helper/macro can be preferable to exposing member-function pointer syntax at every call site.
- Avoid abstractions that hide responsibility or make the flow harder to trace. Prefer abstractions that make correct usage obvious without requiring the caller to understand the internal plumbing.
- Keep tags pure. Do not encode runtime identity into gameplay tags.
- Prefer receiving-side disambiguation when event senders must remain semantically pure.
- Avoid namespace-based helper bags for gameplay systems. Prefer focused static classes when a stateless helper type is needed.
- Remove legacy code once the replacement path is stable, unless the old code still carries active content data.

## UMG Widgets

- Prefer required `BindWidget` over `BindWidgetOptional`.
- Use `BindWidgetOptional` only for decorative, debug-only, or genuinely optional UI elements.
- Core functional widgets should fail loudly if missing from a Widget Blueprint.
- Do not add null guards for required widget bindings just to fail silently. Missing required widgets should be found during editor/test iteration.
- Do not add redundant `check()` calls for required `BindWidget` fields just to revalidate the binding. The required binding itself is the contract.
- Required widget classes, owner references, and initialization inputs should not be hidden behind quiet runtime returns unless the value is genuinely optional.
- Buttons, panels, primary images, and labels required for behavior should be required bindings.
- The goal is to catch missing widget bindings in the editor or compile flow instead of silently degrading at runtime.
- Widgets should own presentation-specific behavior such as displayed text, visual refresh, and binding-change reactions.
- Gameplay components should pass minimal source data to widgets, not format UI text or subscribe to UI refresh events on behalf of the widget.
- Prefer intent-revealing widget APIs such as `SetInputAction(...)` and `ClearInputAction()` over null-argument sentinel calls.

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

## State Ownership

- Do not preserve or restore state unless this system is explicitly borrowing state owned by another active system.
- If a component owns a visual state, model it as direct ownership rather than borrowed state.
- Prefer simple setters/toggles for owned visual state instead of backup structs, maps, or previous-state caches.
- Add lifecycle APIs such as `AddTarget()` and `RemoveTarget()` only when persistent ownership or reuse genuinely needs them.
- Add restoration caches only when a real conflict exists with another system that owns the same state.

Example:

- A highlight component that owns highlighting should directly toggle `RenderCustomDepth`.
- It should not store previous `RenderCustomDepth` or stencil values unless another active feature needs those values restored.

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
- Prefer passing the concrete runtime target to compact utility functions instead of storing hidden setup state when that makes the call site clearer.
- Use default parameters for common defaults when they remove noise without hiding meaning.
- If a small reusable component is needed, keep its API narrow and explicit. For example, a highlight component should apply/restore highlight state, while callers decide which primitive component and stencil value to use.

## Encoding

- C++ source and header files should use `UTF-8 with BOM`.
- Markdown and other documentation files should use `UTF-8`.
- Korean comments are allowed, but they must remain valid UTF-8 text. Do not leave mojibake or partially broken Korean comments in code.
- If an encoding conversion is needed, prefer doing it separately from gameplay/code logic changes so review noise stays low.
- When a Korean code comment is already broken, rewrite it in normal Korean or replace it with a concise English comment instead of preserving the broken text.
