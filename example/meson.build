sources = [
  'main.c',
]

dependencies = []

if build_machine.system() == 'linux'
  dependencies += meson.get_compiler('c').find_library('m')
endif

dependencies += subproject('flecs').get_variable('flecs_dep')
dependencies += flecs_luajit_dep

c_args = []

if get_option('default_library') == 'shared'
  c_args += '-DFLECS_LUAJIT_IMPORTS'
endif

flecs_luajit_example_target = executable(
  'example',
  sources,
  c_args: c_args,
  dependencies: dependencies,
)
