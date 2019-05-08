foundation_test_src = [
    'foundation/testimmutablemap.cpp',
    'foundation/testtaskqueue.cpp'
]

morph_test = executable(
    'morphtest',
    foundation_test_src,
    include_directories: root_include_dir,
    dependencies: [foundation_dep, gtest_dep, thread_dep]
)

test('morphtest', morph_test)
