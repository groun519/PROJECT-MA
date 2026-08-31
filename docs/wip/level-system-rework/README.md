# Level System Rework - Design Workspace

> Branch: `feature/psw/level-system-rework`
> Status: Implementation-ready design draft
> Engine baseline: Unreal Engine 5.8
> Purpose: ChatGPT design sessions and Codex implementation work share one feature-level specification source.

## 1. Purpose

This folder is the temporary design workspace for the level-system rework on `feature/psw/level-system-rework`.

The documents are organized by feature, not by author or implementation phase. Each document must record:

- why the feature exists;
- confirmed requirements and non-requirements;
- rejected alternatives and the reason for rejection;
- the selected Unreal Engine 5.8 implementation path;
- deterministic generation and multiplayer ownership;
- failure/fallback behavior;
- debug and acceptance criteria;
- implementation feedback discovered by Codex.

The target is not maximum extensibility. The target is the minimum structure that supports foreseeable requirements without making invalid map seeds possible.

## 2. Current document set

1. `01_MapGeneration.md`
   - global generation order, deterministic seeds, N x N site distribution, Voronoi generation, cell validation and fallback
2. `02_Area.md`
   - logical Area/Cell distinction, gameplay role, auxiliary/high-ground cells, area-level data contract
3. `03_Connection.md`
   - adjacency, traversal backbone, auxiliary-cell bridging, gate selection, reachability guarantee
4. `04_Path.md`
   - path centerline, corridor polygon, junction-by-union, path-over-terrain priority
5. `05_Floor.md`
   - Dynamic Mesh floor/terrain construction, high ground, cut, tunnel, collision, NavMesh, drop points
6. `06_Environment.md`
   - environment anchors, weight fields and environment blending
7. `07_PCG.md`
   - planned/optional dedicated PCG document; until created, the authoritative PCG handoff contract is in `01`, `04`, and `05`
8. `08_RuntimeLevelChange.md`
   - planned runtime-level-change stress-test document

## 3. Design status labels

- **Fact**: verified project/engine fact.
- **Requirement**: must be satisfied.
- **Expected**: likely requirement; current structure must not block it.
- **Possible**: possible future requirement; must not justify present overengineering.
- **Assumption**: not yet verified.
- **Candidate**: under comparison.
- **Decision**: selected after review.
- **Open Question**: intentionally unresolved.

## 4. Global design rules

1. Every generated `MapSeed` must produce a usable map. Do not solve generation by rerolling seeds.
2. Geometry validity should be valid-by-construction where practical; validation is insurance, not the normal generator.
3. `Path` has higher priority than terrain expression. Terrain may downgrade; required traversal may not disappear.
4. Core gameplay topology is C++ data. PCG consumes the result and must not decide connectivity, boss placement, or whether a floor exists.
5. `Cell`, `Area`, `Connection`, `Path`, and physical `Floor` are data/meaning layers; none of them require one Actor per item.
6. Do not replicate raw Dynamic Mesh vertex buffers as the normal multiplayer strategy.
7. The server owns gameplay generation and traversal decisions.
8. No Tick-based level-generation logic.
9. If one abstraction can be removed without losing a requirement, remove it.
10. Unreal Engine 5.8 is the implementation baseline. Do not add 5.4 compatibility code.

## 5. Confirmed high-level direction

### Spatial generation

- Create `N x N` logical site slots.
- Place exactly one deterministic jittered site in each slot.
- Generate bounded Voronoi cells from those sites.
- The logical grid controls site distribution only; it is not the final visible room boundary.
- Unequal cell size is a feature, not a generation error.

### Cell usage

- A cell large enough for regular combat can be a normal gameplay Area.
- A small cell is an auxiliary candidate rather than an invalid cell.
- Auxiliary cells may become high ground, pits, blocked rock/vegetation regions, monster staging regions, or transit terrain.
- Large cells may be marked as eligible for larger/elite content, but size alone does not force difficulty.

### Traversal

- Required gameplay Areas must be connected.
- Small/auxiliary cells are not automatically traversable.
- If excluding auxiliary cells disconnects required Areas, the connection builder may route through the minimum necessary auxiliary cells.
- A required Path may pass through high ground or otherwise blocked terrain.
- Terrain then expresses that Path as a cut, tunnel, causeway, or simpler fallback.

### Physical geometry

- Canonical spatial data is polygon/boundary data, not PCG Surface data.
- PCG Surface is a derived sampling representation.
- Runtime physical floor/terrain is generated with Dynamic Mesh / Geometry Script.
- Player movement uses physical collision and does not depend on NavMesh.
- Monsters use NavMesh.
- High-ground monsters may patrol on upper NavMesh, watch adjacent lower Areas, then use a one-way drop/jump link to enter combat below.

## 6. Tier classification

### Tier 1 - implement/support now

- finite procedural field;
- deterministic MapSeed;
- N x N bounded jittered sites;
- bounded Voronoi cell generation;
- all-seed fallback without reroll;
- gameplay/auxiliary cell classification;
- guaranteed connectivity of required gameplay Areas;
- path corridor that can cross auxiliary terrain;
- Dynamic Mesh floor/terrain generation;
- high-ground terrain;
- cut-path fallback;
- tunnel support where geometry is simple and valid;
- monster NavMesh on lower/upper surfaces;
- one-way high-ground drop data/link;
- PCG consumes generated spatial data;
- debug drawing and batch seed validation.

### Tier 2 - do not block, do not overbuild

- multi-cell Area merging;
- boss arena structural transformations;
- runtime floor destruction/restoration;
- runtime Connection enable/disable;
- richer tunnel networks;
- environment-specific terrain profiles;
- local dirty-region rebuilds.

### Tier 3 - do not design around now

- player traversal on upper high ground;
- arbitrary multi-floor player navigation;
- freeform voxel terrain;
- large streaming worlds;
- continuous runtime topology generation.

## 7. Codex collaboration contract

Before implementation, Codex should inspect existing Level/AI code and map the semantic types in these documents to existing equivalent types.

Rules:

- Reuse an existing equivalent type instead of creating a duplicate Manager/System.
- Do not rename existing scripts/files unless a genuinely new file is required.
- Do not silently change a `Decision`.
- If an Unreal API differs from the document, record the exact API mismatch and use the nearest 5.8-supported equivalent.
- Prefer one orchestration path and plain Struct arrays over many UObject/Actor layers.
- Add tests/debug commands before adding visual polish.

The implementation order is defined in `01_MapGeneration.md`; `04_Path.md` and `05_Floor.md` define the geometry contracts that Codex should follow without re-designing them.
