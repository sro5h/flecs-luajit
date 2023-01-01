local flecs = require 'flecs'
local world = flecs.world

local ffi = require 'ffi'

ffi.cdef [[
    typedef struct Position { ecs_f32_t x, y; } Position;
]]

flecs.luajit.once(function()
    world:component {
        entity = world:entity { name = 'Position', symbol = 'Position' },
        ctype = 'Position',
    }
end)

flecs.u = {}
flecs.u.Position = world:lookup('Position')

flecs.luajit.once(function()
    local e = world:entity { name = 'Bob' }
    world:set(e, flecs.u.Position, { 10, 20 })
end)

local e = world:lookup('Bob')
print(world:name(e) .. ' on stage ' .. flecs.luajit.stage_id)
local p = world:get(e, flecs.u.Position)
print('  {' .. p.x .. ',' .. p.y .. '}')

print('init.lua executed')
