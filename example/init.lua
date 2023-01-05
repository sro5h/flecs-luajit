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

    world:struct {
        entity = world:entity { name = 'Velocity' },
        members = {
            { name = 'x', type = flecs.g.f32 },
            { name = 'y', type = flecs.g.f32 },
        },
    }
end)

flecs.u = {}
flecs.u.Position = world:lookup('Position')
flecs.u.Velocity = world:lookup('Velocity')

world:cdef(flecs.u.Velocity)

flecs.luajit.once(function()
    local e = world:entity { name = 'Bob' }
    world:set(e, flecs.u.Position, { 10, 20 })
    world:set(e, flecs.u.Velocity, { 42, 37 })
end)

local e = world:lookup('Bob')
local p = world:get(e, flecs.u.Position)
local v = world:get(e, flecs.u.Velocity)

print(world:name(e) .. ' on stage ' .. flecs.luajit.stage_id)
print('  Position = {' .. p.x .. ',' .. p.y .. '}')
print('  Velocity = {' .. v.x .. ',' .. v.y .. '}')

function HelloWorld(iter)
    print('Hello world :)')
end

flecs.luajit.once(function()
    world:system {
        entity = world:entity {
            name = 'HelloWorld',
            add = { flecs.aux.id(flecs.g.DependsOn, flecs.g.OnUpdate) },
        },
        query = { filter = { expr = "" } },
    }
end)

function HelloAgain(iter)
    print('Hello again :p')
end

function OnSetPosition(iter)
    for e in iter:each() do
        print('Setting Position of ' .. iter:world():name(e))
    end
end

print('init.lua executed')
