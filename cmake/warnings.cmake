function(setup_project_warnings target treat_as_errors)
  target_compile_options(${target} INTERFACE
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${treat_as_errors}>:-Werror>
    -Wconversion -Wsign-conversion
  )
endfunction()
