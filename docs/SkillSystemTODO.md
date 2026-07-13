# Skill System TODO

## Asset / JSON Migration

- Later, move skill definitions and related data assets toward the planned JSON-backed structure.
- Validate duplicate addon types while loading JSON, then keep runtime addon traversal allocation-free and make typed addon lookup return immediately when the requested type is found.
- If assembly-time addon lookup becomes worth optimizing, collect assembly-relevant addon references once per source module in `FMASkillAssembler` and pass them to the existing feature assemblers. Keep addons as data and do not move assembly routing into addon virtual functions.
- Keep optional addon state in `FMASkillModuleAddonRuntimeData` instead of expanding `UMASkillModuleInstance` with feature-specific fields. Its current single replicated `TArray<FInstancedStruct>` is acceptable while Stack is the only frequently changing runtime value.
- Revisit addon runtime replication when a second frequently changing runtime addon is introduced. At that point, use item-level delta replication such as `FFastArraySerializer` so only the changed runtime-data item is marked dirty, the client can identify its struct type in the replication callback, and only that addon's payload mirror/listeners are refreshed. Split this notification from the broad `OnStateChanged` path, and avoid replicated addon subobjects unless profiling demonstrates that they are necessary.
- After submodule composition is implemented, replace `AMonster::ApplyPatternRowToActiveSlot()`'s transient Definition duplication and SequenceAddon mutation with a pattern-data builder that materializes Windup as a parameterized submodule. Keep `AMonster` responsible only for pattern selection and application requests so the later JSON transition stays isolated to the builder.
- When old skill/area decal assets are removed or fully resaved under the new data path, remove the temporary CoreRedirects for `MAOverlapDecalDataRow` and `OverlapDecalDataTable` from `Config/DefaultEngine.ini`.
- Revisit visual element ownership during the JSON migration. Damage-time feedback should keep using `DamageTypeTag`, while visuals that are not tied to a damage application moment, such as movement-damage trail decals and montage notify VFX, should resolve from the source module's visual tags first and only fall back to the assembled skill visual tag.
