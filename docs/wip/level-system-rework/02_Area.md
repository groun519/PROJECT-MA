# 02. Area

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. Purpose

Area is the logical gameplay meaning attached to generated spatial space.

The spatial generator first creates bounded Voronoi `Cell` data. Area then describes how that cell participates in gameplay.

Core separation:

> Cell = generated spatial partition.
> Area = gameplay meaning/state and encounter capacity.
> Floor = physical walkable/non-walkable geometry.

## 2. Current implementation decision

### Decision

Initial implementation uses a one-cell-per-Area mapping.

Reasons:

- it is the minimum structure needed for current generation;
- it keeps Voronoi adjacency directly usable;
- it avoids premature cell-merge code;
- small cells can still exist as Auxiliary Areas instead of being deleted.

Keep the data shape capable of referencing CellIDs if an existing Area struct already does so, but do not implement multi-cell merge only for hypothetical flexibility.

### Expected later

Multi-cell Area may be introduced for a guaranteed large boss/event space if gameplay proves it necessary. That is not required for the current Voronoi/terrain implementation.

## 3. Area roles

Gameplay role and physical terrain role are separate.

### Gameplay `AreaRole` candidates

Minimum useful categories:

- `Normal`
- `Auxiliary`

Existing project roles such as Elite/Boss may remain where already used.

Large-cell geometry may set an `EliteEligible`/selection flag, but geometry alone does not force an encounter role.

### Physical `TerrainRole` candidates

- `Ground`
- `HighGround`
- `Pit`
- `Blocked`

Do not combine these enums into one large role enum.

Examples:

- Auxiliary + HighGround
- Auxiliary + Pit
- Normal + Ground
- Auxiliary + HighGround + TransitRequired

## 4. Small cells

### Decision

A small cell is not invalid generation.

Initial classification is based on actual world-space area:

- area >= `MinCombatCellArea` -> gameplay candidate;
- area < threshold -> auxiliary candidate.

Do not use "below X percent of average" as the authoritative gameplay criterion.

### No automatic corrections

Do not normally:

- regenerate the seed;
- move the site;
- relax Voronoi;
- merge the small cell;
- delete the cell.

Use the geometry by assigning a suitable role.

## 5. All-seed gameplay-count fallback

If the current gameplay threshold leaves fewer than the configured minimum number of gameplay Areas:

1. sort cells by polygon area descending;
2. promote the largest non-promoted cells until the minimum count is met;
3. record the promotion for debug;
4. continue with the same MapSeed.

This is deterministic and does not alter spatial geometry.

## 6. Auxiliary Area uses

Auxiliary Areas may be used as:

- high-ground monster staging/patrol space;
- pit;
- cliff/rock block;
- dense environment block;
- narrow transit terrain;
- purely decorative subspace.

Not every auxiliary Area needs a player Path.

If Connection repair requires an auxiliary Area, set `bRequiredForTransit = true`; terrain must then preserve a Path corridor through it.

## 7. High-ground Area gameplay contract

### Player

Current Requirement:

- player does not need to walk on high-ground top;
- player may walk through a lower cut/tunnel corridor when a required Path crosses the Area;
- player movement uses collision, not NavMesh.

### Monster

High-ground monster use:

1. spawn/idle on upper surface;
2. patrol on upper NavMesh;
3. observe adjacent lower gameplay Areas;
4. actual detection uses perception/LOS rather than adjacency alone;
5. on detection, move to a valid drop point;
6. jump/drop to lower combat floor;
7. continue normal ground combat AI;
8. no requirement to climb back up.

This makes the upper connection one-way for current gameplay.

## 8. Area adjacency

`NeighborAreaIDs` are derived from Cell shared-boundary adjacency.

Point-only contact is not adjacency.

An Auxiliary Area can be a neighbor even if it has no active Connection.

Keep:

- spatial adjacency;
- selected gameplay Connection;
- current traversability

as separate concepts.

## 9. Recommended Area data

Reuse existing equivalent project structs when possible.

Semantic minimum:

- `AreaID`
- `CellID` or `CellIDs`
- `AreaRole`
- `TerrainRole`
- `NeighborAreaIDs`
- `ConnectionIDs`
- `bRequiredForTransit`
- `PolygonArea`
- `SpaceBudget`
- gameplay selection flags
- runtime state only if an existing system already needs it

Do not store generated PCG instances or full Dynamic Mesh data inside Area.

## 10. Area centroid and boundary

For the initial one-cell-per-Area implementation:

- Boundary = Cell boundary polygon;
- Centroid = polygon centroid;
- `PolygonArea` = Cell polygon area.

The centroid is also the initial Path anchor because bounded Voronoi cells are convex and the centroid lies inside the polygon.

If multi-cell Areas are later introduced, Area boundary/centroid/area must be recomputed from the merged polygon rather than assuming the first Cell.

## 11. Encounter space capacity

### Decision

Area determines only how much physical monster footprint the space should be allowed to contain.

It does **not** decide:

- which monster types are selected;
- how strong those monsters are;
- skill-module threat evaluation;
- map/stage difficulty scaling.

Those belong to the later Encounter/Monster design.

### Space budget

Use the Area polygon area directly as the capacity source.

Initial formula:

`SpaceBudget = PolygonArea * MonsterOccupancyRatio`

`MonsterOccupancyRatio` is a tunable occupancy allowance. The initial implementation should prefer one global/default value rather than adding per-Area complexity before gameplay requires it.

### Important non-requirement

Do not subtract every derived terrain/detail region from the budget in the initial implementation.

In particular, do not make Area capacity depend on continuously recalculating:

- Path area;
- PCG rocks/vegetation;
- skill ranges;
- temporary combat effects;
- minor terrain presentation changes.

The occupancy ratio is the intended coarse control for keeping enough free combat space.

If later testing proves that a major terrain role makes the simple polygon-area estimate materially wrong, that case may introduce a specific correction. Do not generalize that complexity now.

## 12. Monster space-cost contract

### Decision

Encounter capacity uses a continuous physical measure, not an integer slot count.

A monster's `SpaceCost` is based on its collision footprint projected onto the XY plane.

Therefore:

- `SpaceCost` may be a floating-point value;
- larger collision footprints naturally consume more Area capacity;
- changing attack range or skill shape does not directly change `SpaceCost`;
- `SpaceCost` represents physical occupancy, not combat strength.

The exact footprint formula belongs to the Monster/Encounter implementation, because different collision primitives may require different area calculations.

### Capacity constraint

An Encounter assembled for an Area must satisfy the coarse capacity rule:

`Sum(Monster.SpaceCost) <= Area.SpaceBudget`

This is a composition constraint, not proof that every selected monster already has a valid spawn transform.

## 13. Spawn validation boundary

Actual placement is a later spawn-system responsibility.

After an Encounter composition fits the Area capacity, spawn placement still validates the concrete world positions using the current gameplay requirements, such as collision and valid navigation/ground placement where applicable.

Therefore:

> Polygon area answers "roughly how much monster footprint belongs in this Area?"
>
> Spawn validation answers "can this specific monster be placed at this specific position?"

Do not make `SpaceBudget` attempt to solve exact packing.

## 14. Difficulty and threat boundary

### Decision

Area capacity and combat difficulty are separate axes.

- Area geometry/occupancy determines `SpaceBudget`.
- Map/stage difficulty may change monster type, stats, skill composition, or a later threat budget.
- Two Encounters with similar physical occupancy may have very different difficulty.

The method used to calculate monster combat threat is intentionally unresolved in this document.

Because PROJECT-MA monsters are expected to use the same skill-module system as other combatants, a later Monster/Encounter design may evaluate assembled module data to derive threat automatically. That possibility is recorded but is **not part of Area implementation**.

## 15. Elite/Boss relation to area size

### Decision

Area size is a placement/capacity input, not an encounter decision.

A sufficiently large Area may be marked `EliteEligible` and naturally exposes a larger `SpaceBudget`, allowing an Encounter system to spend more physical capacity on larger or more numerous monsters.

However:

- large Area != automatically Elite;
- Area does not decide the elite monster composition;
- Area does not calculate elite combat strength.

Boss selection similarly must satisfy future boss-space requirements, but is not "largest cell = boss" by rule.

If no random cell satisfies a future boss minimum-size requirement, resolve that feature with a deterministic Area allocation/merge rule; do not reject the MapSeed.

## 16. Floor independence

Area exists even if its physical terrain expression changes.

Examples:

- AreaRole remains Auxiliary while TerrainRole changes HighGround -> Ground fallback;
- Connection remains selected while a tunnel expression falls back to a cut;
- future runtime floor destruction must not delete Area identity.

This separation is the main reason Area remains logical data rather than a Floor Actor.

## 17. Multiplayer

Server owns:

- AreaRole;
- TerrainRole where it affects traversal/collision;
- TransitRequired;
- `SpaceBudget` when it affects authoritative Encounter composition;
- encounter/gameplay state.

Avoid one replicated Actor per Area.

## 18. Validation

For every Area:

- valid AreaID;
- valid Cell reference;
- one initial Area per Cell;
- role values valid;
- boundary non-empty;
- `PolygonArea > 0`;
- `SpaceBudget >= 0`;
- `SpaceBudget` matches the configured occupancy rule within numeric tolerance;
- Neighbor IDs reference real shared-boundary neighbors;
- if `bRequiredForTransit`, at least one selected Connection/Path uses the Area;
- a TerrainRole that blocks base ground must not remove a required Path corridor.

Encounter validation later additionally checks:

- total selected monster `SpaceCost` does not exceed the Area `SpaceBudget`;
- actual spawn transforms pass spawn-system validation.

## 19. Debugging

Display:

- AreaID/CellID;
- `PolygonArea`;
- `MonsterOccupancyRatio` used for calculation;
- `SpaceBudget`;
- AreaRole;
- TerrainRole;
- TransitRequired;
- Neighbor count;
- selected Connections;
- upper patrol/drop information for HighGround.

## 20. Codex implementation contract

Codex should not create a separate Area Actor system for this rework unless the current project already has an Area Actor that must be reused.

The minimum implementation is Struct data owned by the generated map/level container.

For encounter capacity, Codex only needs to expose/store the polygon area and derived `SpaceBudget`. Do not implement monster threat evaluation as part of this Area task.

Do not implement multi-cell Area merge in this phase unless existing required gameplay code already depends on it.
