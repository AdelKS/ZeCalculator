if not meson.is_subproject()
  install_headers(
    files(
      'cpp_function.h',
      'dyn_math_object.h',
    ),
    subdir: fs.relative_to(meson.current_source_dir(), meson.project_source_root() / 'include'),
    preserve_path: true
  )
endif
