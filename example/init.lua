local ecs = require 'flecs'

local ffi = require 'ffi'
ffi.cdef 'typedef struct Position { ecs_float_t x, y; } Position;'

function on_load()
    local Position = ecs.world:component('Position')

    local Velocity = ecs.world:struct({
        entity = ecs.world:entity({ name = 'Velocity' }),
        members = {
            { name = 'x', type = ecs.g.f32 },
            { name = 'y', type = ecs.g.f32 },
        },
    })

    ecs.world:ctype(Velocity)

    local e1 = ecs.world:entity({ name = 'Bob' })
    ecs.world:set(e1, Position, { 11, 42 })

    local e2 = ecs.world:entity({ name = 'Alice' })
    ecs.world:set(e2, Position, { 73, 21 })

    local e3 = ecs.world:entity({ name = 'Otto' })
    ecs.world:set(e3, Velocity, { 27, 91 })

    print('on_load()')
end

function on_load_stage(i)
    local Velocity = ecs.world:lookup('Velocity')

    ecs.world:ctype(Velocity)

    local e = ecs.world:lookup('Otto')
    local v = ecs.world:get(e, Velocity)

    print('[e] ' .. ecs.world:name(e))
    print('  {' .. v.x .. ',' .. v.y .. '}')

    print('on_load_stage(' .. i .. ')')
end

print('init.lua executed')
