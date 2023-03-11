flecs-luajit
============

Introduction
------------
A LuaJIT script host for the [flecs][1] entity component system. Keep in mind
that the project is under development and the API might change unexpectedly.

Example
-------
First the [flecs][1] module has to be imported and initialised
```c
#include <flecs-luajit/module_base.h>
#include <flecs.h>

int main(void) {
    ecs_world_t* world = ecs_init();

    ECS_IMPORT(world, FlecsLuajitBase);
    ecs_luajit_init(world);

    ecs_luajit_run(world, &(ecs_luajit_run_desc_t) {
        .script = { .name = "example.lua" },
    });

    ecs_progress(world, 0.0f);
    return ecs_fini(world);
}
```
An `example.lua` script that imitates the example in the [flecs][1] readme
```lua
local flecs = require 'flecs'
local world = flecs.world

local ffi = require 'ffi'
ffi.cdef('typedef struct { ecs_f32_t x, y; } Position, Velocity;')

local Position = world:component {
    entity = world:entity { name = 'Position', symbol = 'Position' },
}

local Velocity = world:component {
    entity = world:entity { name = 'Velocity', symbol = 'Velocity' },
}

local e = world:entity { name = 'Bob' }
world:set(e, Position, { x = 10, y = 20 })
world:set(e, Velocity, { x = 1, y = 2 })

function Move(iter)
    for _, p, v in iter:each() do
        p.x = p.x + v.x
        p.y = p.y + v.y
    end
end

world:system {
    entity = world:entity {
        name = 'Move',
        add = { flecs.pair(flecs.g.DependsOn, flecs.g.OnUpdate) },
    },
    query = { filter = { expr = "Position, [in] Velocity" } },
    callback = 'Move',
}
```
Note that to make this example work with a multithreaded `ecs_world_t` one has
to make some modifications to the Lua script. A more elaborate example can be
found in the `example` directory.

Compatibility
-------------
The following table shows the compatibility of different versions of
`flecs-luajit` with those of `flecs`

| flecs-luajit | flecs  |
|--------------|--------|
| master       | v3.1.3 |
| v0.1.0       | v3.1.3 |

Building
--------
For now only linux is supported but windows support will come shortly. The
project can be built using the `meson` build system. E.g.
```
meson build
meson compile -C build
```
To run the example application
```
./build/example/example
```

[1]: https://github.com/SanderMertens/flecs
