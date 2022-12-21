get_target_property(LINKER_SCRIPT tm_common::linker_script INTERFACE_SOURCES)

set(LOAD_ADDR 0x88700000)

set(PAYLOADEX_150_DIR ../../../150/ipl_payload/payloadex)

configure_file(
    "${LINKER_SCRIPT}"
    "${CMAKE_CURRENT_BINARY_DIR}/linker.x"
)

add_executable(payloadex_153_01g)
add_executable(tmctrl::payloadex_153_01g ALIAS payloadex_153_01g)
target_sources(payloadex_153_01g PRIVATE ${PAYLOADEX_150_DIR}/main.cpp)
target_link_libraries(payloadex_153_01g PRIVATE tm_common::crt c utils iplsdk::cache iplsdk::ff_ro iplsdk::lowio iplsdk::ms tm_common::common)
target_link_options(payloadex_153_01g PRIVATE -nostdlib -T${CMAKE_CURRENT_BINARY_DIR}/linker.x)

add_custom_command(
    TARGET payloadex_153_01g
    POST_BUILD
    COMMAND psp-objcopy -O binary payloadex_153_01g payloadex_153_01g.bin
)
