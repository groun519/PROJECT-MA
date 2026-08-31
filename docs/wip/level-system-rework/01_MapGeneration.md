# 01. Map Generation

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. Purpose

Map Generation creates the reproducible logical/spatial data for one finite combat field.

It does not decide decorative PCG results and it does not need to own every world Actor. Its output must be sufficient to answer:

- which bounded spatial cells exist;
- which cells are gameplay Areas and which are auxiliary;
- which cells touch;
- which required Areas are connected;
- where the Path centerlines/corridors are;
- which terrain role each cell receives;
- which polygons Floor/PCG/Nav systems should consume.

## 2. Decisions replacing the previous Grid-first draft

### Decision

The previous `shared grid corner -> distorted quadrilateral -> cell merge` approach is no longer the primary implementation.

The primary spatial generator is:

`N x N logical site slots -> deterministic jittered sites -> bounded Voronoi cells`

The logical grid remains only as a stable site-distribution scheme.

### Reason

This keeps the deterministic/debuggable advantages of a grid while producing irregular shared boundaries and naturally different cell sizes. Small cells can be consumed as auxiliary terrain instead of being treated as invalid generation.

## 3. Non-negotiable generation guarantees

### Requirement

- Every integer `MapSeed` must end in a usable generated result.
- Do not reroll a MapSeed because a spatial or terrain expression is inconvenient.
- The generated map must stay inside a finite `MapBounds`.
- Required gameplay Areas must be reachable.
- Terrain may never invalidate a required Path.
- Generation failure in an optional representation must downgrade that representation, not fail the map.
- If the Voronoi engine operation itself cannot produce a valid result, use a deterministic fallback layout for the same seed/settings.

## 4. Engine implementation basis

### Fact - UE 5.8

Use GeometryCore `FDelaunay2` for the primary Voronoi calculation.

Relevant API:

- header: `CompGeom/Delaunay2.h`
- `FDelaunay2::Triangulate(...)`
- `FDelaunay2::CanComputeVoronoiCells()`
- `FDelaunay2::GetVoronoiCells(...)`

`GetVoronoiCells` is called after triangulation and can include boundary cells clipped to a supplied `FAxisAlignedBox2d`.

### Decision

Do not implement a custom Voronoi solver unless an engine defect is proven.

Do not use PCG as the source of gameplay topology.

## 5. Minimum data contract

Names are semantic suggestions. Codex should reuse existing equivalent project types when they already exist.

### `FLevelGenerationSettings`

Minimum fields:

- `GridSizeX`
- `GridSizeY`
- `CellSize`
- `SiteJitterRatio`
- `MinCombatCellArea`
- `ExtraConnectionChance`
- `PathWidth`
- `HighGroundHeight`
- `TunnelClearance`
- `GeometryTolerance`

### `FLevelCellData`

Minimum fields:

- `CellID`
- `LogicalCoord`
- `SitePosition`
- `Centroid`
- `BoundaryPolygon`
- `NeighborCellIDs`
- `AreaRole`
- `TerrainRole`
- `bRequiredForTransit`
- derived measurements such as `Area`

### `FGeneratedLevelData`

Minimum arrays:

- Cells
- Connections
- Paths / Path corridors
- terrain polygon results
- drop-point/link data
- generation/debug metadata

Do not create an Actor/UObject per Cell.

## 6. Deterministic random streams

### Decision

Use a stable root `MapSeed` and derived streams so that changing random-call count in one phase does not reshuffle unrelated systems.

Suggested semantic streams:

- `SiteSeed`
- `ConnectionSeed`
- `TerrainSeed`
- `EnvironmentSeed`
- `DecorationSeed`

The exact hash helper may follow an existing project convention.

`FDelaunay2::RandomStream` should also be initialized deterministically when used.

## 7. Phase 1 - Build logical site slots

Create a regular `GridSizeX x GridSizeY` set of logical slots.

Each slot has:

- stable integer coordinate;
- stable `CellID` seed ordering;
- world-space slot rectangle;
- center point.

No visible geometry is produced at this phase.

## 8. Phase 2 - Jitter one site inside each slot

### Decision

One site is generated per slot.

`Site = SlotCenter + deterministic 2D jitter`

Hard requirement:

`SiteJitterRatio < 0.5`

Initial recommended clamp:

`0.0 <= SiteJitterRatio <= 0.45`

This keeps each site inside its own logical slot and structurally prevents two adjacent slots from producing the exact same site through normal jitter.

### Validation

- site lies inside its assigned slot;
- site lies inside `MapBounds`;
- all site IDs are unique;
- pair distance is above `GeometryTolerance`.

If a numerical duplicate is detected despite configuration, apply a tiny deterministic tie-break offset based on `CellID`; do not reroll the seed.

## 9. Phase 3 - Generate bounded Voronoi cells

1. Convert site positions to `FVector2d`.
2. Initialize `FDelaunay2`.
3. Call `Triangulate(Sites)`.
4. Check triangulation result and `CanComputeVoronoiCells()`.
5. Call `GetVoronoiCells(Sites, true, MapBounds, 0)`.
6. Store one polygon per site in site/CellID order.
7. Normalize polygon winding and remove duplicate-near-equal consecutive vertices using `GeometryTolerance`.

### Required validity

- number of valid cells equals site count;
- every polygon has at least three unique vertices;
- polygon area is positive;
- every vertex is inside/on `MapBounds` within tolerance;
- no polygon has a self-intersection;
- total cell area approximately equals `MapBounds` area.

Voronoi cells are convex under this construction. Keep them convex at this stage; decorative boundary noise belongs to a later visual layer if it is still needed.

## 10. Phase 3 fallback - deterministic regular cells

If triangulation/Voronoi validation fails at engine/numerical level:

- do not choose a new seed;
- create one rectangle from each logical site slot;
- preserve the same `CellID` and logical coordinates;
- record `bUsedSpatialFallback = true`;
- continue the normal pipeline.

The fallback exists to satisfy the all-seed contract. It should be rare and visible in debug logs.

## 11. Phase 4 - Build true cell adjacency

### Decision

Two cells are neighbors only if their final bounded polygons share a boundary segment with length greater than `GeometryTolerance`.

Do not treat point-only contact as adjacency.

Delaunay edges may be used to produce candidate pairs, but the final neighbor relation is confirmed from the actual Voronoi polygons. This avoids incorrect assumptions in cocircular/degenerate triangulation cases.

### Output

For each neighbor pair, cache:

- `CellA`
- `CellB`
- shared boundary endpoints
- shared boundary midpoint
- shared boundary length

This shared-edge cache is reused by Connection/Path generation.

## 12. Phase 5 - Classify cell usability

### Decision

Do not classify a cell as invalid simply because it is smaller than average.

Use world-space gameplay criteria.

Initial implementation may start with the cheapest reliable metric:

- polygon area >= `MinCombatCellArea` -> regular gameplay candidate;
- below threshold -> auxiliary candidate.

Do not add aspect-ratio/inradius analysis until a playtest proves area-only classification insufficient.

### Guarantee for minimum gameplay count

Configuration must not be allowed to make the entire map unusable.

If fewer than the configured minimum gameplay-cell count pass the threshold, deterministically promote the largest cells until the minimum is reached.

No seed reroll.

## 13. Phase 6 - Create Area overlay

Current implementation uses one generated Cell as one initial Area.

- gameplay candidate -> regular Area candidate;
- auxiliary candidate -> Auxiliary Area;
- larger cells may receive an `EliteEligible` flag;
- size never automatically forces Elite difficulty.

Keep `CellIDs` as a collection in Area data if an existing Area type already supports it, but do not implement multi-cell merge in this phase solely for future flexibility.

See `02_Area.md`.

## 14. Phase 7 - Build guaranteed traversal graph

Connection generation runs after gameplay/auxiliary classification.

Conceptual order:

1. Build the graph induced only by required gameplay Areas.
2. Find its connected components.
3. If there is one component, build the selected backbone inside it.
4. If there are multiple components, use the full cell adjacency graph to connect components through the minimum necessary auxiliary cells.
5. Mark auxiliary cells on chosen bridge routes as `bRequiredForTransit`.
6. Add selected adjacent edges as Connections.
7. Add optional extra Connections by deterministic probability.

The full bounded Voronoi adjacency graph is expected to be connected for valid distinct sites over one connected `MapBounds`. Final validation still verifies it.

If the full graph is unexpectedly disconnected because of numerical corruption, invoke the deterministic spatial fallback rather than rerolling.

See `03_Connection.md`.

## 15. Phase 8 - Generate Path centerlines and corridors

For every selected adjacent Connection:

- `AnchorA` = polygon centroid of Cell A;
- `Gate` = midpoint of the confirmed shared boundary;
- `AnchorB` = polygon centroid of Cell B.

Initial centerline polyline:

`AnchorA -> Gate -> AnchorB`

Because each Voronoi cell is convex, the centroid-to-gate segment stays inside that cell. This gives a simple valid-by-construction centerline.

Convert the centerline to a width-bearing 2D corridor polygon.

See `04_Path.md`.

## 16. Phase 9 - Select terrain roles

Only after the traversal graph/path intent is known, assign/confirm TerrainRole.

Examples:

- Ground
- HighGround
- Pit
- Blocked/Decoration

Auxiliary cells are candidates for non-ground terrain, not mandatory non-ground terrain.

### Global priority

`Required Path > TerrainRole`

TerrainRole may change its physical expression based on the path that crosses it.

## 17. Phase 10 - Resolve Path/Terrain interaction

Examples:

### Ground + Path

Keep normal base floor and expose the path to PCG/material systems.

### HighGround + no Path

Generate raised high-ground top and boundary/cliff surfaces.

### HighGround + Path

Try in this order:

1. Tunnel, if tunnel eligibility tests pass.
2. Cut/canyon if tunnel is not eligible or its polygon construction fails.
3. Ground fallback if cut construction also fails.

### Pit + Path

Keep the required corridor traversable at base level; visual expression may be a causeway/bridge later.

The path is not removed.

See `05_Floor.md`.

## 18. Phase 11 - Build physical geometry

Generate runtime floor/terrain with `UDynamicMesh` / `UDynamicMeshComponent` and Geometry Script/GeometryCore polygon operations.

Physical mesh generation must consume the already-decided polygons. It must not change the logical Connection graph.

## 19. Phase 12 - Collision and monster navigation

After physical geometry is ready:

- enable/update collision;
- update dynamic NavMesh for monsters;
- lower ground, tunnel floor and high-ground top may all contribute navigation geometry;
- player movement continues to use physical collision, not NavMesh;
- generate one-way high-ground drop-link data where valid.

## 20. Phase 13 - PCG handoff

PCG receives derived representations such as:

- cell/Area boundary;
- path corridor;
- terrain role;
- high-ground/cut/tunnel masks;
- environment weights.

PCG may turn closed splines/polygons into Surface data for sampling.

PCG does not decide whether required traversal exists and does not own the authoritative physical floor.

## 21. Multiplayer ownership

### Decision

Server:

- owns MapSeed;
- runs authoritative topology/role/path decisions;
- owns gameplay collision/traversal state;
- owns monster navigation/drop-link behavior.

Avoid normal replication of raw Dynamic Mesh vertex arrays.

Preferred replication unit is compact generated data/state or existing project-level deterministic generation data, subject to Codex review of the current networking code.

## 22. Failure hierarchy

Normal fallback order:

1. visual/terrain variant fails -> downgrade variant;
2. tunnel fails -> cut;
3. cut fails -> ground;
4. combined polygon triangulation fails -> triangulate simpler constituent polygons;
5. Voronoi engine/validation fails -> deterministic logical-grid cell fallback;
6. never reroll `MapSeed`.

A PCG decoration failure must not make gameplay generation fail.

## 23. Debug requirements

Provide a level-generation debug view capable of showing:

- MapSeed and derived phase seed;
- logical slot rectangles;
- site points and CellID;
- Voronoi polygons;
- cell area/role;
- shared boundaries;
- selected Connections;
- transit-required auxiliary cells;
- path centerlines and corridor polygons;
- terrain roles;
- tunnel vs cut decisions;
- Dynamic Mesh triangle counts;
- drop points;
- whether any fallback was used.

A reported seed must be reproducible exactly.

## 24. Acceptance tests

Minimum automated/batch validation target before considering this part stable:

### Determinism

For a fixed settings object, generate seeds `0..9999` twice and compare logical output:

- site count/order;
- CellID/order;
- connection pairs;
- area/terrain roles;
- path control points;
- fallback flags.

### Validity

For every generated seed:

- no seed reroll;
- cell count equals requested site count;
- each cell polygon is valid or deterministic spatial fallback is active;
- total cell coverage matches MapBounds within tolerance;
- required gameplay Areas are reachable;
- every Connection references an actual shared boundary;
- every required Path has a non-empty corridor;
- terrain does not erase required traversal;
- Dynamic Mesh generation returns usable collision geometry or terrain downgrades safely.

### Stress cases

Explicitly include:

- Jitter = 0;
- maximum allowed Jitter;
- smallest supported grid;
- largest currently supported grid;
- threshold that creates many auxiliary cells;
- corner cells with multiple small neighbors;
- high-ground transit;
- tunnel ineligible -> cut fallback;
- forced Voronoi fallback test hook.

## 25. Codex implementation order

Codex should implement in this order and compile/test after each numbered block:

1. inspect/reuse existing Level data containers;
2. add generation settings and deterministic seed helpers;
3. generate logical slots/sites;
4. integrate `FDelaunay2` bounded Voronoi;
5. add cell validation and regular-grid fallback;
6. build shared-boundary adjacency cache;
7. add gameplay/auxiliary classification;
8. add Connection backbone and auxiliary bridge repair;
9. add centroid/gate Path centerlines;
10. add corridor polygon generation;
11. add terrain role and tunnel/cut decision data;
12. add Dynamic Mesh floor/terrain builder;
13. add collision and dynamic Nav integration;
14. add high-ground drop-link data;
15. add PCG handoff adapters;
16. add debug draw and seed batch validation.

Do not start with PCG decoration or monster behavior polish before steps 1-12 are stable.
