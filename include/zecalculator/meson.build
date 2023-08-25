subdir('external')
subdir('utils')
subdir('parsing')
subdir('evaluation')
subdir('math_objects')

if not meson.is_subproject()
  install_headers(
    files(
      'mathworld.h',
      'zecalculator.h',
    ),
    subdir: 'zecalculator',
    preserve_path: true
  )
endif
