project('enchant', ['c', 'vala', 'cpp'],
  version: '2.8.19',
  license: 'LGPL-2.1+',
  meson_version: '>= 0.60.0',
  default_options: [
    'c_std=gnu11',
    'cpp_std=c++11',
  ],
)

major_version = meson.project_version().split('.')[0]
minor_version = meson.project_version().split('.')[1]
micro_version = meson.project_version().split('.')[2]

# libtool-style version-info: CURRENT:REVISION:AGE
sonum = major_version.to_int()
age = minor_version.to_int()
current = sonum + age
revision = micro_version.to_int()
# Compiler detection
cc = meson.get_compiler('c')

# Feature test macros needed by gnulib on Linux/glibc
if host_machine.system() == 'linux'
  add_project_arguments('-D_GNU_SOURCE', language: 'c')
  add_project_arguments('-D_GNU_SOURCE', language: 'cpp')
endif

# Generate config.h with feature detection for gnulib
conf = configuration_data()

conf.set('STDC_HEADERS', 1)
conf.set('PACKAGE', '"enchant"')
conf.set('PACKAGE_VERSION', '"@0@"'.format(meson.project_version()))

# Common headers to check
foreach h : [
    'sys/file.h', 'sys/types.h', 'sys/stat.h',
    'unistd.h', 'strings.h', 'stdint.h', 'inttypes.h',
    'locale.h', 'fcntl.h', 'dlfcn.h', 'memory.h',
  ]
  if cc.has_header(h)
    conf.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# Functions needed by gnulib modules
foreach f : [
    'flock', 'setlocale', 'canonicalize_file_name',
    'realpath', 'readlink', 'strdup', 'strchrnul',
    'rawmemchr', 'memset', 'memcpy', 'memmove',
  ]
  if cc.has_function(f)
    conf.set('HAVE_' + f.to_upper(), 1)
  endif
endforeach

conf.set('HAVE_DECL_STRDUP', 1)
conf.set('HAVE_DECL_GETOPT', 1)

# REPLACE macros: 0 = use system version, 1 = use gnulib replacement.
# On modern systems the system versions work correctly.
conf.set('REPLACE_FLOCK', 0)
conf.set('REPLACE_GETOPT', 0)
conf.set('REPLACE_MALLOC', 0)
conf.set('REPLACE_REALLOC', 0)
conf.set('REPLACE_STRDUP', 0)
conf.set('REPLACE_STRCHRNUL', 0)
conf.set('REPLACE_RAWMEMCHR', 0)
conf.set('REPLACE_CANONICALIZE_FILE_NAME', 0)
conf.set('REPLACE_SETLOCALE', 0)

# Gnulib module indicator macros.
# These activate the corresponding gnulib implementations in the .in.h
# replacement headers.  Set to 1 for every module enchant imports.
conf.set('GNULIB_BCP47', 1)
conf.set('GNULIB_FLOCK', 1)
conf.set('GNULIB_GETOPT', 1)
conf.set('GNULIB_UNISTD_H_GETOPT', 1)
conf.set('GNULIB_GETOPT_POSIX', 1)
conf.set('GNULIB_MALLOC', 1)
conf.set('GNULIB_REALLOC', 1)
conf.set('GNULIB_STRCHRNUL', 1)
conf.set('GNULIB_STRDUP', 1)
conf.set('GNULIB_RAWMEMCHR', 1)
conf.set('GNULIB_STDLIB_H_NONNULL', 1)
conf.set('GNULIB_STDLIB_H_SAFER', 1)
conf.set('GNULIB_STRING_H_NONNULL', 1)
conf.set('GNULIB_STRING_H_SAFER', 1)
conf.set('GNULIB_UNISTD_H_NONNULL', 1)
conf.set('GNULIB_UNISTD_H_SAFER', 1)

# Type size checks
conf.set('SIZEOF_INT', cc.sizeof('int'))
conf.set('SIZEOF_LONG', cc.sizeof('long'))
conf.set('SIZEOF_LONG_LONG', cc.sizeof('long long'))
conf.set('SIZEOF_SIZE_T', cc.sizeof('size_t'))
conf.set('SIZEOF_VOID_P', cc.sizeof('void*'))

# MSVC invalid parameter handler (needed by gnulib msvc-inval.h on Windows)
if host_machine.system() == 'windows' and cc.get_id() == 'msvc'
  conf.set('HAVE_MSVC_INVALID_PARAMETER_HANDLER', 1)
else
  conf.set('HAVE_MSVC_INVALID_PARAMETER_HANDLER', 0)
endif

# struct flock member check (needed by gnulib flock.c)
if cc.has_member('struct flock', 'l_type', prefix: '#include <fcntl.h>')
  conf.set('HAVE_STRUCT_FLOCK_L_TYPE', 1)
else
  conf.set('HAVE_STRUCT_FLOCK_L_TYPE', 0)
endif

# Generate config.h from template
configure_file(
  input: 'config.h.meson.in',
  output: 'config.h',
  configuration: conf,
)

# Include root build dir so config.h is found by all subdirectories
add_project_arguments('-I' + meson.current_build_dir(), language: ['c', 'cpp'])
if cc.get_id() == 'msvc'
  add_project_arguments('/FIconfig.h', language: ['c', 'cpp'])
else
  add_project_arguments('-include', 'config.h', language: ['c', 'cpp'])
endif

glib_dep = dependency('glib-2.0', version: '>= 2.76')
gmodule_dep = dependency('gmodule-2.0')
gobject_dep = dependency('gobject-2.0')
gio_dep = dependency('gio-2.0')

if host_machine.system() == 'windows'
  shlibs_in_bindir = true
else
  shlibs_in_bindir = false
endif

subdir('libgnu')
subdir('lib')
subdir('src')
subdir('providers')
subdir('tests')
subdir('program-tests')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name: 'libenchant',
  description: 'A spell checking library',
  version: meson.project_version(),
  filebase: 'enchant-@0@'.format(major_version),
  requires_private: ['glib-2.0', 'gmodule-no-export-2.0'],
  libraries: libenchant,
  subdirs: 'enchant-@0@'.format(major_version),
)
