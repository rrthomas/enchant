vala_src = files(
  'api.vala',
  'broker.vala',
  'composite.vala',
  'dict.vala',
  'provider.vala',
  'provider-dict.vala',
  'pwl.vala',
  'util.vala',
)

enchant_c_args = [
  '-D_ENCHANT_BUILD=1',
  '-DENCHANT_VERSION_STRING="@0@"'.format(meson.project_version()),
  '-DENCHANT_MAJOR_VERSION="@0@"'.format(major_version),
  '-DG_LOG_DOMAIN="libenchant"',
]

libenchant = shared_library(
  'enchant-@0@'.format(major_version),
  vala_src,
  vala_args: [
    '--debug',
    '--no-color',
    '--vapidir', meson.current_source_dir(),
    '--pkg', 'configmake',
    '--pkg', 'internal',
    '--pkg', 'posix',
    '--pkg', 'gnu',
    '--pkg', 'gio-2.0',
    '--pkg', 'gmodule-2.0',
  ],
  c_args: enchant_c_args,
  link_with: libgnu,
  dependencies: [glib_dep, gmodule_dep, gio_dep, gobject_dep, gnulib_dep],
  # -no-undefined is needed on Windows with MinGW (GCC) where unresolved
  # symbols are allowed by default in shared libraries. MSVC requires all
  # symbols to be resolved by default, and does not support this flag.
  link_args: (host_machine.system() == 'windows' and cc.get_id() != 'msvc') ? ['-no-undefined'] : [],
  # soversion = libtool CURRENT = MAJOR + MINOR = 2 + 8 = 10
  # version = CURRENT.REVISION.AGE
  soversion: current,
  version: '@0@.@1@.@2@'.format(current, revision, age),
  install: true,
  install_dir: get_option('libdir'),
)

enchant_dep = declare_dependency(
  link_with: libenchant,
  include_directories: include_directories('.'),
  dependencies: [glib_dep, gmodule_dep, gio_dep, gobject_dep, gnulib_dep],
)

install_headers(
  'enchant.h',
  'enchant++.h',
  install_dir: get_option('includedir') / 'enchant-@0@'.format(major_version),
)

# Install data files
install_data(
  'enchant.ordering',
  install_dir: get_option('datadir') / 'enchant-@0@'.format(major_version),
)
