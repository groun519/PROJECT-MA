# 04. Path

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. Purpose

Path turns selected Connections into physical traversal corridors and PCG exclusion/response regions.

The current design intentionally avoids a separate complex PathGraph.

## 2. Core decisions

### Decision 1 - one deterministic centerline per selected Connection

For adjacent Areas A/B:

- `AnchorA` = Area polygon centroid;
- `Gate` = shared-boundary midpoint;
- `AnchorB` = Area polygon centroid.

Initial centerline:

`AnchorA -> Gate -> AnchorB`

### Decision 2 - width becomes a 2D corridor polygon

The centerline itself is not the final traversable region.

Convert the open path/polyline to a closed width-bearing polygon and use that polygon for:

- terrain subtraction/override;
- Floor construction;
- PCG blocking/masking;
- debug validation.

### Decision 3 - Path has priority over terrain

A TerrainRole may change from Tunnel to Cut or to Ground, but a required Path is not discarded.

## 3. Why centroid-to-gate is the initial implementation

Bounded Voronoi cells are convex.

For a valid convex cell:

- polygon centroid is inside the cell;
- Gate lies on its boundary;
- the straight segment centroid -> Gate lies inside/on the cell.

Therefore the centerline is valid-by-construction for the two adjacent cells.

This removes the need for an A* path solver inside each Area for the initial implementation.

## 4. Junction behavior without Junction objects

If Area A has three selected Connections, three centerline arms reach the same Area centroid.

After path-width offset and polygon Union, the overlapping corridors naturally form a junction region.

Initial implementation:

- no Junction Actor;
- no Junction UObject;
- no pairwise Gate-to-Gate path set;
- no separate PathGraph.

If the resulting hub is visually too sharp, add a small deterministic centroid pad polygon before Union. This is a geometry refinement, not a new logical system.

## 5. UE 5.8 polygon implementation

### Fact

Geometry Script 5.8 provides polygon-list operations including:

- Create Polygons From Path Offset;
- Polygons Union;
- Polygons Difference;
- Polygons Intersection;
- Polygons Offset.

### Decision

Core canonical path data remains plain generated points/polygons.

Use Geometry Script polygon functions as an implementation adapter for robust offset/boolean operations rather than writing a custom polygon boolean library.

## 6. Corridor generation sequence

For each selected Connection:

1. create 2D polyline `[AnchorA, Gate, AnchorB]`;
2. remove zero-length consecutive segments within `GeometryTolerance`;
3. offset the open path by `PathWidth / 2`;
4. receive one or more closed corridor polygons;
5. clip/intersect with `MapBounds` if needed;
6. validate positive area;
7. add to the global path polygon list.

After all selected Connections:

8. Union corridor polygons;
9. optionally Union small centroid hub pads;
10. store/cache final `PathFieldPolygonList`.

## 7. Path width and narrow Voronoi cells

### Decision

Do not fail because a Cell or shared edge is narrower than `PathWidth`.

The centerline remains valid. The corridor may consume much of the small cell or overlap nearby terrain regions.

Terrain generation must yield to this required corridor.

Consequences:

- very small HighGround + Path may become mostly cut;
- a tunnel may become ineligible and fall back to cut;
- a tiny auxiliary terrain feature may disappear completely;
- none of these invalidate the Path.

## 8. Path through HighGround

Terrain resolver chooses:

### Tunnel

Keep upper high-ground top while preserving lower path floor.

Use only when tunnel eligibility passes.

### Cut

Subtract/intersect the path corridor from the high-ground terrain expression so the lower route remains open.

### Ground fallback

If terrain polygon construction still fails, downgrade the affected high-ground region to Ground.

The selected Connection and centerline do not change.

## 9. Path through Pit/Blocked terrain

Current minimal rule:

- the required corridor remains base-level traversable Floor;
- surrounding terrain may remain pit/blocked;
- visual bridge/causeway dressing is separate and may be added later.

Do not make bridge asset availability a topology requirement.

## 10. Path data ownership

Recommended semantic data:

- `PathID` or ConnectionID association;
- centerline points;
- width;
- final corridor polygon(s);
- optional terrain interaction result.

A separate Spline Component is not required for every Path.

If PCG integration is easier with `USplineComponent`, create/maintain only the representation components needed by the PCG adapter rather than making them the authoritative path data.

## 11. PCG contract

PCG consumes the final path field.

Uses include:

- remove/reduce trees and rocks;
- path-edge vegetation;
- material/path weight;
- tunnel/cut entrance decoration.

PCG does not decide the Path.

Where a PCG Surface is useful, derive it from a closed spline/polygon representation of the already-generated corridor.

## 12. Floor contract

Path provides 2D corridor data to Floor/Terrain construction.

Typical operations:

- `Ground` cell: base Floor already exists; Path may only affect material/PCG.
- `HighGround Cut`: subtract Path corridor from high-ground polygon.
- `HighGround Tunnel`: intersect Path corridor with the cell to obtain tunnel footprint.
- `Pit/Blocked`: intersect Path corridor with the cell and add/retain base Floor for that footprint.

See `05_Floor.md`.

## 13. Geometry failure rules

### Required fallback order

1. single corridor offset returns invalid -> build a simple constant-width segment/capsule fallback for that Connection;
2. global Union fails -> retain validated constituent corridor polygons as a list;
3. terrain boolean against Path fails -> terrain downgrades;
4. never remove the Connection/Path because a decorative polygon operation failed.

Geometry failure must be logged with MapSeed, ConnectionID and input points.

## 14. Validation

For each selected Connection:

- centerline has at least two non-equal points;
- Gate belongs to actual shared boundary;
- centroid-to-gate segments are inside/on their respective convex cell within tolerance;
- corridor area > tolerance;
- corridor intersects both connected Areas;
- corridor stays inside MapBounds after clip;
- final PathField contains a traversable region for the Connection.

For all required gameplay Areas:

- their selected Connection graph is reachable before physical mesh generation.

## 15. Debugging

Display toggles:

- centerline;
- centroid anchors;
- Gate points;
- raw per-Connection corridor;
- final Union path field;
- corridor width;
- terrain interaction type: Ground/Tunnel/Cut/Fallback.

## 16. Performance

Generation is event-based.

Do not:

- resample/boolean Paths every Tick;
- create one Actor per Path;
- recompute the same corridor independently in PCG and Floor systems.

Cache the generated polygon representation and share/convert it through adapters.

## 17. Codex implementation contract

Codex should implement the straight centroid-gate-centroid path first.

Do not add curve control points, A*, custom path smoothing, or dedicated Junction classes in the first implementation.

After acceptance tests pass, visual curvature may be added as a second step provided:

- endpoints/Gate remain fixed;
- curve remains inside safe spatial bounds or terrain still yields;
- deterministic generation remains intact.
