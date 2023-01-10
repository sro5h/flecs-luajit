- Use `ecs_enable_id` to enable and disable `(Loaded, Loadable)` pair instead
  of adding and removing it
- Add `EcsLuajitCDef` tag to indicate that a meta struct should be automatically
  registered in the lua states
- Use plecs module in example to define an `EcsLuajitSystem` (and the entities
  and components)
- Stop initialisation if any of the internal scripts (i.e. boot.lua etc.) fail
- Rename `boot.lua` script to `module_base.lua` (what about `glue.lua`?)
