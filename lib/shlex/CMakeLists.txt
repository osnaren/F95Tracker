set(shlexcpp_sources "${SHLEXCPP_DIR}/shlex.cpp" "${SHLEXCPP_DIR}/shlex.h")
set(shlex_sources "${SHLEX_DIR}/shlex.cpp" "${SHLEX_DIR}/shlex.h")
set(sources "${shlex_sources}" "${shlexcpp_sources}")

add_library(shlex STATIC "${sources}")
