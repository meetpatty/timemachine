get_target_property(LINKER_SCRIPT iplsdk::linker_script INTERFACE_SOURCES)

# No need to set stack pointer in rebootex
set(STACK_SIZE 0x0)

set(LOAD_ADDR 0x88FC0000)

set(REBOOTEX_150_DIR ../../../150/tmctrl/rebootex)

configure_file(
    "${LINKER_SCRIPT}"
    "${CMAKE_CURRENT_BINARY_DIR}/linker.x"
)

add_executable(rebootex153_01g)
add_executable(tmctrl::rebootex153_01g ALIAS rebootex153_01g)
target_sources(rebootex153_01g PRIVATE ${REBOOTEX_150_DIR}/crt0.s ${REBOOTEX_150_DIR}/main.cpp)
target_link_libraries(rebootex153_01g PRIVATE c utils iplsdk::cache iplsdk::ff_ro iplsdk::lowio iplsdk::ms iplsdk::syscon tm_common::common)
target_link_options(rebootex153_01g PRIVATE -nostdlib -T${CMAKE_CURRENT_BINARY_DIR}/linker.x)

add_custom_command(
    TARGET rebootex153_01g
    POST_BUILD
    COMMAND psp-objcopy -O binary rebootex153_01g rebootex153_01g.bin
)
