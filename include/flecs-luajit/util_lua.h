#ifndef FLECS_LUAJIT_UTIL_LUA_H
#define FLECS_LUAJIT_UTIL_LUA_H

#include <lua.h>

#define luaX_stack_guard_prolog(l) \
        int luaX_stack_guard__top = lua_gettop((l))

#define luaX_stack_guard_epilog(l, d) \
        assert(lua_gettop((l)) == (luaX_stack_guard__top + (d)))

#endif
