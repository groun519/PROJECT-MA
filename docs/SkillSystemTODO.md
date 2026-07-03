# Skill System TODO

## Asset / JSON Migration

- Later, move skill definitions and related data assets toward the planned JSON-backed structure.
- When old skill/area decal assets are removed or fully resaved under the new data path, remove the temporary CoreRedirects for `MAOverlapDecalDataRow` and `OverlapDecalDataTable` from `Config/DefaultEngine.ini`.
- Revisit visual element ownership during the JSON migration. Damage-time feedback should keep using `DamageTypeTag`, while visuals that are not tied to a damage application moment, such as movement-damage trail decals and montage notify VFX, should resolve from the source module's visual tags first and only fall back to the assembled skill visual tag.
