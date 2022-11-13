#ifndef FLECS_LUAJIT_CONFIG_H
#define FLECS_LUAJIT_CONFIG_H

// TODO: Make portable
#if defined(FLECS_LUAJIT_EXPORTS)
#       define FLECS_LUAJIT_API __attribute__((visibility("default")))
#elif defined(FLECS_LUAJIT_IMPORTS)
#       define FLECS_LUAJIT_API __attribute__((visibility("default")))
#else
#       define FLECS_LUAJIT_API
#endif

#endif
