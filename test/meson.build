subdir('src')

test_sources = files(
    'parser_test.cpp',
)

cmake = import('cmake')

boost_ut_proj = cmake.subproject('boost-ut')
boost_ut_inc = boost_ut_proj.include_directories('ut')

foreach test_src: test_sources
    test_exe = executable(
        fs.stem(test_src), # name executable after source file without .cpp extension
        test_src, # source file path
        include_directories: boost_ut_inc,
        dependencies : [zecalculator_dep, zecalculator_testing_dep] # dependencies, here the vcl headers
    )

    test(fs.stem(test_src), test_exe) # register as a test
endforeach
