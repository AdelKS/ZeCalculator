subdir('src')

test_sources = files(
    'ast_test.cpp',
    'data_test.cpp',
    'dyn_math_object_test.cpp',
    'evaluation_test.cpp',
    'expression_test.cpp',
    'fast_test.cpp',
    'function_test.cpp',
    'global_variable_test.cpp',
    'math_world_test.cpp',
    'readme_example_test.cpp',
    'rpn_test.cpp',
    'sequence_test.cpp',
    'tokenizer_test.cpp',
    'utils.cpp',
)

foreach test_src: test_sources
    test_exe = executable(
        fs.stem(test_src), # name executable after source file without .cpp extension
        test_src, # source file path
        dependencies : zecalculator_testing_dep # dependencies, here the vcl headers
    )

    test(fs.stem(test_src), test_exe, args: ['--success']) # register as a test
endforeach
