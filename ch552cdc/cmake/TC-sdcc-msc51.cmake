list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)
# which compilers to use for C and C++
set(CMAKE_C_COMPILER sdcc)
set(CMAKE_CXX_COMPILER sdcc-c++-unsupported)
set(CMAKE_OBJCOPY sdobjcopy)
set(CMAKE_AR sdar)
set(CMAKE_NM sdnm)
set(CMAKE_RANLIB sdranlib)

set(CMAKE_ASM_COMPILER sdas8051)
set(CMAKE_ASM_COMPILER_ID "SDCC-8051")
set(CMAKE_ASM_FLAGS "-mmcs51")

set(FREQ_SYS 24000000)
set(XRAM_LOC 0x0100)
set(XRAM_SIZE 0x0300)
set(CODE_SIZE 0x3800)

add_compile_options(
    -mmcs51
    --model-small
    --no-xinit-opt
    --xram-size ${XRAM_SIZE}
    --xram-loc ${XRAM_LOC}
    --code-size ${CODE_SIZE}
)
add_compile_definitions(
    -DF_CPU=${FREQ_SYS}
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --debug")
endif()


set(CMAKE_ASM_FLAGS_INIT "")
