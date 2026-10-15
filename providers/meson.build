provider_install_dir = get_option('libdir') / 'enchant-@0@'.format(major_version)

provider_common_c_args = ['-D_ENCHANT_BUILD=1']
provider_inc = include_directories('../lib')

provider_link_with = [libenchant, libgnu]
provider_deps = [glib_dep, gmodule_dep, gio_dep, gobject_dep, gnulib_dep]

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# pkg-config based providers
hunspell_dep = dependency('hunspell', required: get_option('hunspell'))
nuspell_dep = dependency('nuspell', version: '>= 5.1.0', required: get_option('nuspell'))
voikko_dep = dependency('libvoikko', required: get_option('voikko'))
zemberek_dep = dependency('dbus-glib-1', version: '>= 0.62', required: get_option('zemberek'))

# aspell: header + library
aspell_option = get_option('aspell')
has_aspell_h = false
aspell_lib_found = false
if not aspell_option.disabled()
  has_aspell_h = cc.has_header('aspell.h')
  if has_aspell_h
    aspell_lib = cc.find_library('aspell', required: aspell_option)
    aspell_lib_found = aspell_lib.found()
  elif aspell_option.enabled()
    error('aspell.h not found but aspell provider was requested')
  endif
endif

# hspell: header + library
hspell_option = get_option('hspell')
has_hspell_h = false
hspell_lib_found = false
if not hspell_option.disabled()
  has_hspell_h = cc.has_header('hspell.h')
  if has_hspell_h
    hspell_lib = cc.find_library('hspell', required: hspell_option)
    hspell_lib_found = hspell_lib.found()
  elif hspell_option.enabled()
    error('hspell.h not found but hspell provider was requested')
  endif
endif
zlib_dep = cc.find_library('z', required: false)

# WinSpell (Windows-only)
winspell_option = get_option('winspell')
has_spellcheck_h = false
winspell_system_deps = []
if host_machine.system() == 'windows' and not winspell_option.disabled()
  has_spellcheck_h = cxx.has_header('spellcheck.h')
  if has_spellcheck_h
    winspell_system_deps = [
      cxx.find_library('ole32', required: winspell_option),
      cxx.find_library('oleaut32', required: winspell_option),
      cxx.find_library('uuid', required: winspell_option),
    ]
  elif winspell_option.enabled()
    error('spellcheck.h not found but winspell provider was requested')
  endif
endif

# AppleSpell (macOS-only)
applespell_option = get_option('applespell')
has_cocoa_h = false
applespell_found = false
applespell_dep = declare_dependency()
if host_machine.system() == 'darwin' and not applespell_option.disabled()
  has_cocoa_h = cxx.has_header('Cocoa/Cocoa.h')
  if has_cocoa_h
    applespell_dep = dependency('appleframeworks', modules: ['Cocoa'], required: applespell_option)
    applespell_found = applespell_dep.found()
  elif applespell_option.enabled()
    error('Cocoa/Cocoa.h not found but applespell provider was requested')
  endif
endif

# Build provider modules
provider_modules = []

if hunspell_dep.found()
  provider_modules += 'hunspell'
  shared_module('enchant_hunspell',
    files('enchant_hunspell.cpp'),
    cpp_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [hunspell_dep],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if nuspell_dep.found()
  provider_modules += 'nuspell'
  shared_module('enchant_nuspell',
    files('enchant_nuspell.cpp'),
    cpp_args: provider_common_c_args + ['-std=c++17'],
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [nuspell_dep],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if voikko_dep.found()
  provider_modules += 'voikko'
  shared_module('enchant_voikko',
    files('enchant_voikko.c'),
    c_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [voikko_dep],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if has_aspell_h and aspell_lib_found
  provider_modules += 'aspell'
  shared_module('enchant_aspell',
    files('enchant_aspell.c'),
    c_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [aspell_lib],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if has_hspell_h and hspell_lib_found
  provider_modules += 'hspell'
  hspell_extra_deps = []
  if zlib_dep.found()
    hspell_extra_deps += [zlib_dep]
  endif
  shared_module('enchant_hspell',
    files('enchant_hspell.c'),
    c_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [hspell_lib] + hspell_extra_deps,
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if host_machine.system() == 'windows' and has_spellcheck_h
  provider_modules += 'winspell'
  winspell_cpp_args = provider_common_c_args + [
    '-D_WIN32_WINNT=_WIN32_WINNT_WIN8',
    '-DNTDDI_VERSION=NTDDI_WIN8',
  ]
  shared_module('enchant_winspell',
    files('enchant_winspell.cpp'),
    cpp_args: winspell_cpp_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + winspell_system_deps,
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if host_machine.system() == 'darwin' and has_cocoa_h and applespell_found
  provider_modules += 'applespell'
  shared_module('enchant_applespell',
    files('applespell_checker.mm'),
    objcpp_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [applespell_dep],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

if zemberek_dep.found()
  provider_modules += 'zemberek'
  shared_module('enchant_zemberek',
    files('enchant_zemberek.cpp'),
    cpp_args: provider_common_c_args,
    include_directories: provider_inc,
    link_with: provider_link_with,
    dependencies: provider_deps + [zemberek_dep],
    install: true,
    install_dir: provider_install_dir,
    name_prefix: '',
  )
endif

summary('Providers to build', ', '.join(provider_modules), section: 'Providers')
if provider_modules.length() == 0
  warning('No spell-checking provider selected!')
endif
