subdir('decl')
subdir('impl')

if not meson.is_subproject()
  install_headers(
    files(
      'aliases.h',
      'builtin_binary_functions.h',
      'builtin_unary_functions.h',
      'forward_declares.h',
      'function.h',
      'global_constant.h',
      'object_list.h',
      'sequence.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
