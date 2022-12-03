flecs-luajit
============

Introduction
------------
A LuaJIT script host for the [flecs][1] entity component system. This project is intended to be used with [flecs-luajit-binding][2] to bring Lua scripting to [flecs][1]. Among other things it allows the creation of systems from Lua scripts and plecs files.

Example
-----------
The following basic example showcases some of the features of the `flecs-luajit` module used together with `flecs-luajit-binding`.
```c
#include <flecs-luajit/module.h>

int main(void) {
    ecs_world_t* world = ecs_init();
    ecs_set_threads(world, 2);

    ECS_IMPORT(world, FlecsConfigLuajit);
    ecs_singleton_set(world, EcsLuajitConfig, {
        .init_code =
            "require 'flecs-luajit-binding.flecs' {\n"
            "   world = ...,\n"
            "   cdef = require 'flecs-luajit-binding.flecs_cdef'\n"
            "}\n"
        ,
    })

    ECS_IMPORT(world, FlecsLuajit);
    ecs_luajit_ensure_stages(world);

    ecs_plecs_from_str(world, "example",
        "using flecs.meta\n"

        "Struct Position {\n"
        "   x :- { f32 }\n"
        "   y :- { f32 }\n"
        "}\n"

        "PrintPosition {\n"
        "   - luajit.System { query_expr: `[in] Position` }\n"
        "   - (DependsOn, OnUpdate)\n"
        "   - luajit.Script {`\n"
        "       return function(iter)\n"
        "           for e, p in iter:each() do\n"
        "               print(iter:world():name(e))\n"
        "               print('  {' .. p.x .. ',' .. p.y .. '}')\n"
        "           end\n"
        "       end\n"
        "   `}\n"
        "}\n"

        "my_entity {\n"
        "   - Position { x: 10, y: 20 }\n"
        "}\n"
    )

    ecs_progress(world, 0);

    return ecs_fini(world);

    // Output:
    // my_entity
    //   {10,20}
}
```
The same example can be achieved from Lua using an initialisation script. By default `flecs-luajit` looks for a file named 'init.lua' and after executing it tries to call the `on_load` callback. So instead of loading data from a plecs string using `ecs_plecs_from_str` one can create a file named 'init.lua' containing
```lua
local ecs = require 'flecs'

function on_load()
    local Position = ecs.world:struct({
        entity = ecs.world:entity({ 'Position' }),
        members = {
            { 'x', ecs.g.f32 },
            { 'y', ecs.g.f32 },
        },
    })

    local my_entity = ecs.world:entity({ 'my_entity' })
    ecs.world:set(my_entity, Position, { x = 10, y = 20 })

    local PrintPosition = ecs.world:entity({ 'PrintPosition' })
    ecs.world:add(PrintPosition, ecs.g.DependsOn, ecs.g.OnUpdate)
    ecs.world:set(PrintPosition, ecs.g.LuajitSystem, { query_expr = '[in] Position' })
end

function PrintPosition(iter)
    for e, p in iter:each() do
        print(iter:world():name(e))
        print('  {' .. p.x .. ',' .. p.y .. '}')
    end
end
```
Of course both approaches can be mixed freely and can be further combined with usage of the regular C API of `flecs`.

Explanation
-------------
At the most basic level `flecs-luajit` provides a C API to execute Lua code on the internal `lua_State`s and to create systems that execute Lua code instead of the usual C callbacks. On top of that it defines components that allow doing the same things without using the C API directly.  
Internally it creates and initialises a `lua_State` for each stage in the `ecs_world_t`. Then it tries to execute a user provided initialisation script or file. Lastly it calls the `on_load` callback on the `lua_State` associated to the main stage and then the `on_load_stage` callback on every `lua_State` (including the one associated to the main stage). As each stage has its own independent `lua_State` it works even if the world is multithreaded. In particular the Lua systems created using `flecs-luajit` can be multithreaded just as a regular `ecs_system_t` can. Due to the independent nature of the `lua_State`s the intended way to share data between them is through the world.  
Although `flecs-luajit` doesn't depend on `flecs-luajit-binding` and could be used with another LuaJIT binding both are designed to be used together.

[1]: https://github.com/SanderMertens/flecs
[2]: https://github.com/sro5h/flecs-luajit-binding
