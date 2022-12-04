local flecs = require 'flecs'
local ffi = require 'ffi'

function flecs.World:ctype(struct)
    local identifier = self:identifier(struct)

    if identifier == nil or pcall(ffi.typeof, identifier) then
        return
    end

    local cdef = ffi.C.ecs_luajit_struct_cdef(self, struct)

    if cdef == nil then
        return
    end

    ffi.cdef(ffi.string(ffi.gc(cdef, flecs.aux.free)))
end
