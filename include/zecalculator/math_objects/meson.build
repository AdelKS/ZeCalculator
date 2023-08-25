subdir('decl')
subdir('impl')

if not meson.is_subproject()
  install_headers(
    files(
      'builtin_binary_functions.h',
      'builtin_unary_functions.h',
      'expression.h',
      'function.h',
      'global_constant.h',
      'global_variable.h',
      'sequence.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
