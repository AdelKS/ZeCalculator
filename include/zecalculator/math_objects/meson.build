subdir('decl')
subdir('impl')

if not meson.is_subproject()
  install_headers(
    files(
      'aliases.h',
      'builtin.h',
      'cpp_function.h',
      'dyn_math_object.h',
      'forward_declares.h',
      'function.h',
      'global_constant.h',
      'math_object.h',
      'object_list.h',
      'sequence.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
