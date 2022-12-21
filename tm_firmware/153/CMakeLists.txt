project(tm_153 VERSION 1.0 LANGUAGES CXX ASM)

add_compile_definitions(PSP_FW_VERSION=150 TM_PATH="/TM/153" TMCTRL_FILE="/tmctrl153.prx")

add_subdirectory(ipl_payload)
add_subdirectory(tmctrl)
