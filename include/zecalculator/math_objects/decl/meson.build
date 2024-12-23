subdir('internal')

if not meson.is_subproject()
  install_headers(
    files(
      'cpp_function.h',
      'dyn_math_object.h',
      'function.h',
      'global_constant.h',
      'math_object.h',
      'sequence.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
