- Add function to directly execute lua code on a specific stage. Could also
  use a custom component (e.g. `EcsLuajitOnStage`) to indicate a script should
  only be executed on a specific stage. Or a pair of the form
  `(EcsLuajitOnStage, EcsLuajitScript)`
- Assign the returned value of `EcsLuajitScript` to the entity's name
