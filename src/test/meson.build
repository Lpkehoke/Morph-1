foundation_test_src = [
    'foundation/testimmutablemap.cpp',
    'foundation/testtaskqueue.cpp',
    'foundation/testobservable.cpp',
]

core_test_src = [
    'core/testlogger.cpp',
]

morph_test = executable(
    'morphtest',
    foundation_test_src + core_test_src,
    include_directories: root_include_dir,
    dependencies: [foundation_dep, gtest_dep, thread_dep, core_dep, tbb_dep]
)

test('morphtest', morph_test, timeout: 1000)
