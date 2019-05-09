doxygen = find_program('doxygen', required: true)
sphinx = find_program('sphinx-build', required: true)

sphinx_src_dir = 'source'
sphinx_output_dir = 'html'

run_command(doxygen, 'Doxyfile')
run_command(sphinx, sphinx_src_dir, sphinx_output_dir, check: true)
