add_subdirectory(rebootex)

add_executable(tmctrl153)
set(TMCTRL_150_DIR ../../150/tmctrl)
build_prx(tmctrl153 EXPORTS ${TMCTRL_150_DIR}/exports.exp)

target_sources(tmctrl153 PRIVATE ${TMCTRL_150_DIR}/main.c)
target_compile_options(tmctrl153 PRIVATE -fshort-wchar -Wno-address-of-packed-member)
target_link_libraries(tmctrl153 PRIVATE tm_common::moduleutils tm_common::flash_emu pspdisplay_driver_weak pspreg_driver_weak pspsdk pspkernel)

add_custom_command(OUTPUT rebootex.h
    DEPENDS tmctrl::rebootex153_01g
    COMMAND $<TARGET_PROPERTY:bin2cpp,EXEC>
        --gzip --c_style
        rebootex_01g="$<TARGET_FILE:rebootex153_01g>".bin
        rebootex.h
)

target_sources(tmctrl153 PRIVATE rebootex.h)
target_include_directories(tmctrl153 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(tmctrl153 PRIVATE ${TMCTRL_150_DIR}/lib)

add_custom_command(TARGET tmctrl153
    POST_BUILD
    COMMAND psp-fixup-imports tmctrl153.prx
)
