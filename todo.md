- Use `ecs_enable_id` to enable and disable `(Loaded, Loadable)` pair instead
  of adding and removing it
- Register world in lua states and make it work with the binding
- Add c function to generate a c definition string for meta components and use
  it in `ffi.cdef` to register it with lua states
- Could also execute this register function automatically and register nested
  types recursively
- Add more elaborate example actually creating and using entities and components
- Use plecs in example to define an `EcsLuajitSystem` (and the entities and
  components)
- Add lua function to automatically register (all) entities with a name into a
  given table
