package.path = package.path .. ';./external/flecs-luajit-binding/?.lua'
local ecs = require 'flecs' { world = ..., cdef = require 'flecs_cdef' }

local ffi = require 'ffi'
ffi.cdef 'typedef struct Position { ecs_float_t x, y; } Position;'

function on_load()
    local Position = ecs.world:component('Position')

    local e1 = ecs.world:entity({ name = 'Bob' })
    ecs.world:set(e1, Position, { 11, 42 })

    local e2 = ecs.world:entity({ name = 'Alice' })
    ecs.world:set(e2, Position, { 73, 21 })

    print('on_load()')
end

function on_load_stage(i)
    print('on_load_stage(' .. i .. ')')
end

print('init.lua executed')
