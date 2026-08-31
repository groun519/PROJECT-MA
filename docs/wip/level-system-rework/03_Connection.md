# 03. Connection

> Status: Implementation-ready design draft
> Branch: `feature/psw/level-system-rework`
> Engine baseline: Unreal Engine 5.8

## 1. Purpose

Connection is a selected logical traversal edge between two spatially adjacent Areas.

It answers:

- should these adjacent Areas participate in the playable route graph;
- what shared boundary do they cross;
- does an auxiliary Area have to become transit terrain;
- where should Path generation cross the Area boundary.

Connection is not the Path mesh/corridor itself.

## 2. Spatial adjacency vs Connection

### Spatial adjacency

Derived automatically when two bounded Voronoi polygons share a non-zero-length boundary segment.

### Connection

A subset of those adjacency edges selected by the traversal generator.

Therefore:

`Neighbor != Connected`

and later:

`Connected != currently traversable`

remain valid distinctions.

## 3. Shared-boundary cache

Map Generation caches for each adjacent pair:

- Area/Cell A ID;
- Area/Cell B ID;
- shared boundary start/end;
- midpoint;
- length.

Point-only contact is ignored.

This cache is authoritative input for Gate creation.

## 4. Gate decision

### Decision

Initial Gate = midpoint of the confirmed shared boundary.

Do not require the shared boundary itself to be wider than the final Path width.

Reason:

Path corridor is allowed to consume/downgrade surrounding terrain. A narrow shared Voronoi edge is not a reason to invalidate a seed.

The Gate is a centerline crossing anchor, not a literal pre-existing doorway aperture.

### Later variation

Gate position may be moved along the shared edge for variety only after the midpoint implementation is proven stable. That variation must remain deterministic.

## 5. Required graph nodes

Regular gameplay Areas are required nodes.

Auxiliary Areas are optional nodes unless selected as a bridge/transit node.

The goal is:

> all required gameplay Areas belong to one selected traversal component.

It is not required that every auxiliary Area has a Path.

## 6. Connection generation algorithm

### Phase A - direct gameplay graph

1. Build an induced graph containing only gameplay Areas.
2. Edges exist only for actual shared-boundary adjacency.
3. Find connected components.

If one component exists, no auxiliary repair is needed.

### Phase B - connect disconnected gameplay components through auxiliary cells

If multiple components exist:

1. Search the full cell adjacency graph.
2. Find a shortest deterministic route between two disconnected gameplay components.
3. Route cost should prioritize fewer auxiliary cells.
4. Add every adjacency edge on that route to the selected Connection set.
5. Mark auxiliary Areas on the route `bRequiredForTransit = true`.
6. Merge the two components.
7. Repeat until all required gameplay Areas are connected.

Because the full valid bounded Voronoi adjacency graph covers one connected `MapBounds`, a route should exist. Final validation verifies this assumption.

### Deterministic tie-breaking

When multiple equal routes exist, tie-break by a stable `(ConnectionSeed, CellID)` ordering, not container/hash iteration order.

## 7. Backbone inside connected gameplay regions

After connectivity repair, remove unnecessary cycles only if needed by the desired level density.

Initial cheap strategy:

- deterministic randomized DFS/BFS spanning backbone;
- then add extra eligible adjacency edges using `ExtraConnectionChance`.

Exact Euclidean MST is not required.

The goal is controlled route density, not minimum total physical edge length.

## 8. Auxiliary transit behavior

A selected Connection can enter/leave an Auxiliary Area.

That does not change `AreaRole` to Normal.

Instead:

- `bRequiredForTransit = true`;
- Path generation creates a corridor;
- Terrain generation adapts HighGround/Pit/Blocked expression around the corridor.

Thus a small cell can remain visually high/blocked while still containing a cut or tunnel.

## 9. Recommended Connection data

Minimum:

- `ConnectionID`
- `AreaAID`
- `AreaBID`
- shared-boundary/gate reference or cached Gate point
- `bEnabled` only if existing runtime logic requires it
- optional seed/debug data

Do not store full Path polygons in Connection.

Do not create an Actor per Connection.

## 10. Why no separate complex PathGraph is required now

The selected Connection edges already form the graph.

Path geometry can be derived from:

- each Area centroid;
- each Connection Gate.

A cell with several selected Connections naturally becomes a junction when its corridor arms overlap/union around the centroid.

Therefore the current implementation does not need:

- separate Junction UObject;
- separate PathGraph manager;
- all-pairs Gate routing.

If later playtests show poor junction shape, improve geometry without changing logical Connection semantics.

## 11. Reachability validation

After selecting Connections:

1. start BFS/DFS from any required gameplay Area;
2. traverse selected enabled Connections;
3. assert every required gameplay Area is reached.

Also validate:

- no self Connection;
- no duplicate unordered Area pair;
- each Connection pair is an actual shared-boundary adjacency;
- Gate lies on the shared boundary within tolerance;
- any Auxiliary Area used by a selected route is marked TransitRequired.

Failure is a generator bug, not a reason to reroll MapSeed.

## 12. Runtime state

Current generation does not need a large state machine.

If existing gameplay needs runtime blocking, a minimal `bEnabled`/existing equivalent is preferred.

Future doors/bridges/floor destruction may change traversal state without deleting the logical Connection.

Do not implement multiple speculative enums now.

## 13. Multiplayer

Server owns:

- selected Connection set;
- TransitRequired decisions;
- runtime enable/disable state.

Do not rely on client-side random graph generation for authoritative gameplay unless the existing networking architecture explicitly validates deterministic reconstruction.

## 14. Debugging

Draw:

- all spatial adjacency edges in a faint/debug-only representation;
- selected Connections distinctly;
- gameplay connected-component ID before repair;
- auxiliary bridge routes;
- Gate points;
- Area degree;
- reachability result.

A seed that needed auxiliary repair should be obvious in debug view.

## 15. Acceptance cases

Explicit tests:

1. all gameplay cells already connected;
2. one small auxiliary cell separates two gameplay components;
3. several small cells form a barrier chain;
4. corner gameplay cell has only auxiliary neighbors;
5. multiple equal bridge routes;
6. Jitter = 0 cocircular/grid symmetry;
7. maximum allowed Jitter;
8. only minimum allowed gameplay-area count.

Every case must end with required gameplay reachability and no seed reroll.
