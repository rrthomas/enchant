# Program tests: shell scripts that test the command-line tools.
# These use a meson-compatible test runner that sets up the build-tree
# environment instead of doing DESTDIR installs.

enchant_exe_path = enchant_exe.full_path()
enchant_lsmod_exe_path = enchant_lsmod_exe.full_path()

# Create meson-compatible test runner
test_runner = configure_file(
  input: 'run-meson-test.in',
  output: 'run-meson-test',
  configuration: {
    'ENCHANT_EXE': enchant_exe_path,
    'ENCHANT_LSMOD_EXE': enchant_lsmod_exe_path,
    'ENCHANT_MAJOR_VERSION': major_version,
    'SOURCE_DIR': meson.current_source_dir(),
  },
)

# Copy test data to build directory
foreach f : [
    'misspelled-input.txt',
    'misspelled-input-expected.txt',
    'unknown-option-expected.txt',
    'zero-arguments-expected.txt',
    'en.aff',
    'en.dic',
  ]
  configure_file(
    input: f,
    output: f,
    copy: true,
  )
endforeach

# Test environment
test_env = environment()
test_env.set('ENCHANT_MAJOR_VERSION', major_version)

# Each test case as a separate meson test
test('misspelled-input',
  test_runner,
  args: [meson.current_source_dir() / 'misspelled-input.sh'],
  env: test_env,
  workdir: meson.current_build_dir(),
  depends: [enchant_exe],
  is_parallel: false,
)

test('unknown-option',
  test_runner,
  args: [meson.current_source_dir() / 'unknown-option.sh'],
  env: test_env,
  workdir: meson.current_build_dir(),
  depends: [enchant_exe],
  is_parallel: false,
)

test('zero-arguments',
  test_runner,
  args: [meson.current_source_dir() / 'zero-arguments.sh'],
  env: test_env,
  workdir: meson.current_build_dir(),
  depends: [enchant_exe],
  is_parallel: false,
)

test('run-enchant-lsmod',
  test_runner,
  args: [meson.current_source_dir() / 'run-enchant-lsmod.sh'],
  env: test_env,
  workdir: meson.current_build_dir(),
  depends: [enchant_lsmod_exe],
  is_parallel: false,
)
