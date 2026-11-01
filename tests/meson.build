unittestpp_dep = dependency('UnitTest++', version: '>= 1.6', required: false)

if not unittestpp_dep.found()
  message('UnitTest++ >= 1.6 not found: skipping C++ unit tests')
  subdir_done()
endif

tests_inc = include_directories('../lib')

tests_cpp_args = [
  '-DLIBDIR_SUBDIR="."',
  '-DENCHANT_MAJOR_VERSION="@0@"'.format(major_version),
]

# Mock provider modules: shared modules loaded at runtime.
# Automake builds these with -module -avoid-version, which in libtool
# creates files like libenchant_mock_provider.so (with lib prefix still
# present in the build directory). We replicate by using shared_module
# and setting name_prefix to 'lib' for compatibility with the CopyProvider
# helper in EnchantBrokerTestFixture.h.
mock_provider_deps = [glib_dep, gobject_dep, gmodule_dep]
mock_provider_args = tests_cpp_args + ['-D_ENCHANT_BUILD=1']

shared_module('enchant_mock_provider',
  files('mock_provider.cpp'),
  cpp_args: mock_provider_args,
  include_directories: tests_inc,
  dependencies: mock_provider_deps,
  link_with: [libenchant],
  name_prefix: 'lib',
  install: false,
  build_by_default: true,
)

shared_module('enchant_mock_provider2',
  files('mock_provider.cpp'),
  cpp_args: mock_provider_args,
  include_directories: tests_inc,
  dependencies: mock_provider_deps,
  link_with: [libenchant],
  name_prefix: 'lib',
  install: false,
  build_by_default: true,
)

shared_module('enchant_null_provider',
  files('mock_provider.cpp'),
  cpp_args: mock_provider_args + ['-DENCHANT_TEST_NULL_PROVIDER'],
  include_directories: tests_inc,
  dependencies: mock_provider_deps,
  link_with: [libenchant],
  name_prefix: 'lib',
  install: false,
  build_by_default: true,
)

shared_module('enchant_null_identify',
  files('mock_provider.cpp'),
  cpp_args: mock_provider_args + ['-DENCHANT_TEST_NULL_IDENTIFY'],
  include_directories: tests_inc,
  dependencies: mock_provider_deps,
  link_with: [libenchant],
  name_prefix: 'lib',
  install: false,
  build_by_default: true,
)

shared_module('enchant_null_describe',
  files('mock_provider.cpp'),
  cpp_args: mock_provider_args + ['-DENCHANT_TEST_NULL_DESCRIBE'],
  include_directories: tests_inc,
  dependencies: mock_provider_deps,
  link_with: [libenchant],
  name_prefix: 'lib',
  install: false,
  build_by_default: true,
)

# Test executables
main_test_src = files(
  'main.test.cpp',
  'dictionary/add.cpp',
  'dictionary/add_to_session.cpp',
  'dictionary/check.cpp',
  'dictionary/describe.cpp',
  'dictionary/free_string_list.cpp',
  'dictionary/get_error.cpp',
  'dictionary/get_extra_word_characters.cpp',
  'dictionary/is_added.cpp',
  'dictionary/is_removed.cpp',
  'dictionary/is_word_character.cpp',
  'dictionary/remove_from_session.cpp',
  'dictionary/remove.cpp',
  'dictionary/suggest.cpp',
  'broker/describe.cpp',
  'broker/dict_exists.cpp',
  'broker/free_dict.cpp',
  'broker/free.cpp',
  'broker/get_error.cpp',
  'broker/init.cpp',
  'broker/list_dicts.cpp',
  'broker/request_dict.cpp',
  'broker/request_dict_with_pwl.cpp',
  'broker/request_pwl_dict.cpp',
  'broker/set_ordering.cpp',
  'pwl/pwl.cpp',
  'provider/broker_set_error.cpp',
  'provider/dict_set_error.cpp',
  'provider/get_user_config_dirs.cpp',
)

providers_test_src = files(
  'providers/providers.test.cpp',
  'providers/dictionary/check.cpp',
  'providers/dictionary/suggest.cpp',
  'providers/provider/describe_dict.cpp',
  'providers/provider/dictionary_exists.cpp',
  'providers/provider/identify_dict.cpp',
  'providers/provider/list_dicts.cpp',
  'providers/provider/request_dict.cpp',
)

main_test = executable('main.test',
  main_test_src,
  cpp_args: tests_cpp_args,
  include_directories: tests_inc,
  link_with: [libenchant],
  dependencies: [glib_dep, gobject_dep, gmodule_dep, unittestpp_dep],
)

providers_test = executable('providers.test',
  providers_test_src,
  cpp_args: tests_cpp_args,
  include_directories: tests_inc,
  link_with: [libenchant],
  dependencies: [glib_dep, gobject_dep, gmodule_dep, unittestpp_dep],
)

# Test data
configure_file(
  input: 'test.pwl.orig',
  output: 'test.pwl',
  copy: true,
)

# Test environment
test_env = environment()
test_env.set('ENCHANT_CONFIG_DIR', meson.current_build_dir() / 'config')

test('main.test', main_test,
  env: test_env,
  workdir: meson.current_build_dir(),
)

test('providers.test', providers_test,
  env: test_env,
  workdir: meson.current_build_dir(),
)
