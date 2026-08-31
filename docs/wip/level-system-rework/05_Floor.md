# 05. Floor

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. Purpose

Floor/Terrain converts generated 2D spatial decisions into actual render/collision geometry.

Core principle:

> Area/Cell defines logical space.
> Path defines required traversal.
> Terrain defines how non-path space is expressed.
> Dynamic Mesh is the physical representation.

PCG Surface is not the physical floor.

## 2. Selected physical implementation

### Decision

Use runtime `UDynamicMesh` / `UDynamicMeshComponent` for generated floor and terrain geometry.

UE 5.8 `UDynamicMeshComponent` renders geometry backed by a `UDynamicMesh`/`FDynamicMesh3`.

Use Geometry Script / GeometryCore operations for polygon triangulation and mesh construction.

### Required project dependencies

Codex should verify/reuse existing dependencies and add only what is actually required by the selected API path, expected to include:

- `GeometryCore`
- `GeometryFramework`
- `GeometryScriptingCore`
- `NavigationSystem` if directly referenced
- existing `PCG`

Geometry Script is a Runtime plugin in UE 5.8. Do not add 5.4 compatibility branches.

## 3. Component ownership

### Decision

Do not create one Floor Actor per Cell.

Prefer one existing/new level-geometry owner with a small number of Dynamic Mesh Components.

Initial semantic split:

- `GroundMesh`: base traversable floor/corridors;
- `TerrainMesh`: high-ground top/cliff/tunnel structural surfaces.

A third component may be justified only if collision/material/update behavior requires it.

The generated data remains in Struct arrays; the components are presentation/collision owners.

## 4. Canonical 2D inputs

Floor builder consumes:

- MapBounds;
- Cell boundary polygons;
- TerrainRole;
- final Path corridor polygon list;
- HighGroundHeight;
- TunnelClearance/ceiling thickness settings.

Do not ask PCG to reconstruct these shapes.

## 5. Base ground construction

### Ground cells

Regular Ground Areas contribute their full cell polygon to base traversable floor.

### HighGround/Pit/Blocked cells

They do not automatically contribute their full base polygon.

If a required Path crosses them, contribute the required corridor footprint at base level.

Therefore:

`BaseFloor = GroundRegions UNION RequiredCorridorFootprintsInNonGroundRegions`

Implementation may construct this as one Polygon List and triangulate it.

### Fallback

If a combined Polygon List triangulation reports an error:

- keep constituent validated polygons;
- triangulate them individually;
- avoid overlapping duplicate regions by clipping corridor footprints to non-ground cells where practical.

Do not fail the seed.

## 6. UE 5.8 triangulation path

Geometry Script 5.8 supports:

- `Append Triangulated Polygon`;
- `Append Polygon List Triangulation`.

`Append Polygon List Triangulation` reports triangulation errors and supports polygon-list input.

### Decision

Use Polygon List triangulation for combined floor regions where stable.

Use single-polygon triangulation as the simpler fallback.

Ensure polygon winding is normalized before append.

## 7. HighGround without Path

For a HighGround cell that does not contain a required Path:

1. top polygon = cell polygon;
2. generate top surface at `BaseZ + HighGroundHeight`;
3. generate cliff/side surfaces from the outer polygon boundary down toward BaseZ;
4. enable collision on relevant top/side surfaces;
5. allow upper top to contribute monster NavMesh.

Player does not need a route to the upper surface.

## 8. HighGround + Path decision

For every HighGround cell intersected by a required corridor:

1. test Tunnel eligibility;
2. if eligible, build Tunnel;
3. otherwise build Cut;
4. if Cut polygon generation fails, downgrade the cell TerrainRole to Ground.

This is deterministic from generated data/settings.

## 9. Tunnel eligibility

Initial Tunnel should be intentionally conservative.

Tunnel is eligible only when:

- path footprint intersects the cell boundary in a simple entry/exit form;
- exactly two usable boundary crossing regions exist;
- the corridor inside the cell is one connected strip;
- no centroid/junction branching occurs inside the tunnel cell;
- tunnel clearance satisfies player/monster capsule requirements;
- resulting wall/ceiling features are larger than `GeometryTolerance`.

If any condition is ambiguous, use Cut.

This keeps tunnel generation a safe terrain expression rather than a topology dependency.

## 10. Tunnel construction

### Important decision

Do not require a 3D mesh boolean that cuts a solid hill.

Construct tunnel surfaces from 2D regions.

Inputs:

- `CellPolygon`
- `TunnelFootprint = PathCorridor INTERSECTION CellPolygon`

Build:

1. lower tunnel floor from `TunnelFootprint` at BaseZ;
2. upper high-ground top from full `CellPolygon` at `BaseZ + HighGroundHeight`;
3. tunnel side walls from left/right footprint boundary segments inside the cell;
4. tunnel ceiling/underside over the footprint at a configured ceiling Z;
5. outer cliff walls, excluding entry/exit openings.

The volume does not need to be a closed manifold solid if render/collision requirements are satisfied by explicit surfaces.

### Entry/exit

Entry/exit are the portions where the tunnel footprint intersects the cell outer boundary.

Do not place cliff wall faces across these openings.

## 11. Cut construction

For a HighGround cell with a Cut:

`RaisedPolygonList = CellPolygon DIFFERENCE PathCorridor`

Then:

1. triangulate raised polygon(s) at HighGround Z;
2. build side/cliff walls along resulting raised boundaries;
3. keep the required Path footprint at BaseZ.

This creates a canyon/road cut through the raised cell.

If the subtraction leaves tiny fragments below `GeometryTolerance`/minimum terrain feature area, discard those terrain fragments.

If nothing meaningful remains, the cell effectively becomes Ground around the Path. This is valid.

## 12. Pit/Blocked + Path

Current minimal behavior:

- preserve/add base-level Path floor through the cell;
- surrounding Pit/Blocked expression remains;
- optional bridge/causeway visuals are PCG/visual work.

Do not require a special bridge gameplay system for initial connectivity.

## 13. Cliff/wall generation

Initial implementation should use polygon boundary edges directly.

For each boundary segment that requires a vertical wall:

- top edge at terrain Z;
- bottom edge at lower reference Z;
- emit quad as two triangles;
- keep winding/normals consistent.

For Cut terrain, the path-created internal boundary also becomes a cliff/canyon wall.

For Tunnel terrain, suppress outer wall segments at entry/exit and add dedicated tunnel side surfaces.

Do not add a general-purpose 3D CSG framework.

## 14. Collision

### Requirement

Physical floor/wall collision is authoritative for player and gameplay physics.

Player does not use NavMesh.

Dynamic Mesh collision generation may be more expensive than rendering; generation happens as a level-build event, not Tick.

Codex should:

- configure collision only after a mesh batch is built where possible;
- avoid recooking after each appended polygon;
- batch mesh edits and then update collision;
- profile collision cook time in the seed batch/debug output.

## 15. Monster NavMesh

### Decision

Monster navigation uses Unreal Recast NavMesh with runtime geometry updates.

UE 5.8 `ERuntimeGenerationType::Dynamic` supports geometry changes; `DynamicModifiersOnly` is insufficient for newly generated runtime geometry.

Generated navigable surfaces may include:

- lower Ground mesh;
- tunnel floor;
- HighGround top.

The player remains independent from NavMesh.

## 16. High-ground monster patrol contract

Level generation only needs to provide stable spatial/navigation data; combat AI remains in AI code.

For a HighGround Area:

- upper NavMesh supports patrol/random reachable movement;
- adjacent lower gameplay Areas are observation candidates;
- actual player detection uses normal perception/LOS;
- when aggroed, monster chooses a valid one-way drop point/link;
- after landing it transitions to normal ground combat behavior;
- returning to high ground is not required.

## 17. Drop-point generation

Generate candidates along HighGround boundaries that border lower traversable ground.

Candidate requirements:

- source point lies on/near valid upper NavMesh;
- destination lies on valid lower walkable floor;
- destination is not inside a pit/invalid tunnel ceiling region;
- vertical drop is within the monster archetype's allowed drop range;
- landing clearance is sufficient;
- link does not require upward traversal.

Store semantic data such as:

- source;
- destination;
- source AreaID;
- destination AreaID;
- allowed monster/navigation flags.

### Nav link implementation

UE 5.8 `ANavLinkProxy` connects separate NavMesh areas and supports simple/smart links.

Use an existing project equivalent if present.

A Smart Link/custom movement callback is appropriate when the AI must play a jump/drop action rather than teleporting along the link.

Do not spawn links for every boundary edge; create only validated drop candidates actually needed by HighGround gameplay.

## 18. Tunnel and layered NavMesh

The current gameplay permits:

- monsters on HighGround top;
- monsters on lower tunnel floor;
- player only on lower tunnel floor.

The two surfaces may share XY space at different Z.

This is acceptable as physical geometry; validate actual Recast generation in the implementation test map.

No player upper-layer traversal system is required.

## 19. PCG handoff

Derived 2D shapes should be exposed to PCG for decoration:

- Area boundary;
- Path corridor;
- HighGround polygon;
- Cut wall/edge tags;
- tunnel entrance points/regions;
- terrain role.

PCG Surface may be created from closed spline/polygon representations for sampling.

PCG is downstream. If PCG generation fails, Floor/Collision/Connection remain valid.

## 20. Multiplayer

Server owns authoritative physical/traversal state.

Do not normally replicate raw Dynamic Mesh vertices.

Preferred approach:

- replicate compact generated level data/settings/state as appropriate to existing project networking;
- construct client visual mesh from that data;
- server collision remains authoritative.

Codex must inspect current listen-server replication before choosing the exact replication container.

## 21. Terrain fallback hierarchy

Mandatory order:

### Tunnel

Tunnel invalid/ambiguous/boolean failure
-> Cut

### Cut

Difference/triangulation failure
-> Ground

### HighGround optional fragment

Tiny/invalid fragment
-> discard fragment

### Combined floor triangulation

Error
-> constituent polygon triangulation fallback

No terrain representation may cause a MapSeed reroll.

## 22. Validation

For every generated physical map:

- every required Path has continuous collision floor;
- Ground floor triangles lie at expected base Z;
- HighGround top is at expected upper Z;
- no cliff wall closes a tunnel entry/exit;
- Tunnel has sufficient clearance;
- Cut leaves no required corridor blocked;
- terrain fallback state matches the final mesh;
- Dynamic Mesh contains no invalid triangle indices;
- collision exists on required player floor;
- Recast can find expected monster navigation on lower/upper test surfaces;
- each generated drop link has valid source/destination.

## 23. Debugging/profiling

Record/draw:

- polygon input count;
- triangle count per component;
- mesh build time;
- collision cook/update time;
- Nav rebuild time if measurable;
- HighGround/Cut/Tunnel role;
- tunnel eligibility failure reason;
- fallback reason;
- upper/lower NavMesh;
- drop points/links.

## 24. Acceptance scenarios

Create deterministic tests/maps for:

1. normal Ground cell;
2. isolated HighGround cell;
3. HighGround crossed by a simple Path -> Tunnel;
4. HighGround with branch/junction -> Cut fallback;
5. very small HighGround where Cut removes almost all terrain;
6. Pit crossed by required Path;
7. tunnel with upper monster patrol and lower player route;
8. high-ground monster one-way drop;
9. polygon triangulation failure injection -> simpler fallback;
10. 10,000-seed batch with zero MapSeed rerolls.

## 25. Codex implementation contract

Implement in this order:

1. base Ground Dynamic Mesh;
2. HighGround top + outer walls;
3. Path corridor floor through non-ground cells;
4. Cut subtraction and walls;
5. conservative Tunnel;
6. collision batching/update;
7. dynamic NavMesh validation;
8. drop-point/link generation;
9. PCG adapters;
10. profiling and batch validation.

Do not start with arbitrary 3D mesh boolean terrain, multi-level player traversal, or runtime terrain destruction.
