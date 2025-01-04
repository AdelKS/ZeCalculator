subdir('decl')
subdir('impl')

if not meson.is_subproject()
  install_headers(
    files(
      'builtin.h',
      'cpp_function.h',
      'dyn_math_object.h',
      'forward_declares.h',
      'object_list.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
