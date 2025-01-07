project(tm_500 VERSION 1.0 LANGUAGES C CXX ASM)

add_compile_definitions(PSP_FW_VERSION=500 TM_PATH="/TM/DC8" TM_PATH_W=L"\\\\TM\\\\DC8\\\\")

add_subdirectory(common)
add_subdirectory(ipl_payload)
add_subdirectory(tmctrl)
