subdir('utils')
subdir('parsing')
subdir('evaluation')
subdir('math_objects')
subdir('mathworld')

if not meson.is_subproject()
  install_headers(
    files(
      'zecalculator.h',
      'error.h'
    ),
    subdir: fs.relative_to(meson.current_source_dir(), meson.project_source_root() / 'include'),
    preserve_path: true
  )
endif
