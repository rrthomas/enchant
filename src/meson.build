util_lib = static_library(
  'util',
  'util.vala',
  vala_args: [
    '--debug',
    '--no-color',
    '--vapidir', meson.current_source_dir(),
    '--vapidir', meson.project_source_root() / 'lib',
    '--pkg', 'config',
    '--pkg', 'configmake',
    '--pkg', 'posix',
    '--pkg', 'gio-2.0',
    '--pkg', 'gmodule-2.0',
    '--pkg', 'enchant-2',
    '--header', 'util.h',
  ],
   c_args: [
    '-DENCHANT_MAJOR_VERSION="@0@"'.format(major_version),
  ],
  dependencies: [glib_dep, gmodule_dep, gio_dep, gobject_dep, gnulib_dep],
)

common_vala_args = [
  '--debug',
  '--no-color',
  '--vapidir', meson.current_source_dir(),
  '--vapidir', meson.project_source_root() / 'lib',
  '--pkg', 'config',
  '--pkg', 'configmake',
  '--pkg', 'internal',
  '--pkg', 'gnu',
  '--pkg', 'posix',
  '--pkg', 'gio-2.0',
  '--pkg', 'gmodule-2.0',
  '--pkg', 'enchant-2',
]

src_c_args = [
  '-DENCHANT_MAJOR_VERSION="@0@"'.format(major_version),
  '-DENCHANT_VERSION_STRING="@0@"'.format(meson.project_version()),
]

enchant_exe = executable(
  'enchant-@0@'.format(major_version),
  'enchant.vala',
  vala_args: common_vala_args,
  c_args: src_c_args,
  dependencies: [enchant_dep, gnulib_dep],
  link_with: [util_lib],
  install: true,
  install_dir: get_option('bindir'),
)

enchant_lsmod_exe = executable(
  'enchant-lsmod-@0@'.format(major_version),
  'enchant-lsmod.vala',
  vala_args: common_vala_args,
  c_args: src_c_args,
  dependencies: [enchant_dep, gnulib_dep],
  link_with: [util_lib],
  install: true,
  install_dir: get_option('bindir'),
)
