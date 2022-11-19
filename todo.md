- Add component `EcsLuajitScript` and function `ecs_luajit_do` to directly
  execute lua code on a stage
- Add `EcsOnSet` observer for `EcsLuajitScript` that executes it and assigns
  the returned value to the entity's name
