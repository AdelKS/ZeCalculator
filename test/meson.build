subdir('src')

test_sources = files(
    'evaluation_test.cpp',
    'function_test.cpp',
    'math_world_test.cpp',
    'parser_test.cpp',
    'syntax_tree_test.cpp',
)

foreach test_src: test_sources
    test_exe = executable(
        fs.stem(test_src), # name executable after source file without .cpp extension
        test_src, # source file path
        dependencies : zecalculator_testing_dep # dependencies, here the vcl headers
    )

    test(fs.stem(test_src), test_exe) # register as a test
endforeach
