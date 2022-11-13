project(
  'flecs-luajit',
  'c',
  version: '0.1.0',
  license: 'mit',
  default_options: [
    'c_std=c99',
    'warning_level=3'
  ],
  subproject_dir: 'external'
)

srcs = [
  'src/module.c',
]

deps = []

if build_machine.system() == 'linux'
  deps += meson.get_compiler('c').find_library('m')
endif

deps += subproject('flecs').get_variable('flecs_dep')

args = []

if get_option('default_library') == 'shared'
  args += '-DFLECS_LUAJIT_EXPORTS'
endif

flecs_luajit_lib = library(
  'flecsluajit',
  srcs,
  c_args: args,
  dependencies: deps,
)

flecs_luajit_dep = declare_dependency(
  link_with: flecs_luajit_lib,
  include_directories: include_directories('include'),
)

subdir('example')
