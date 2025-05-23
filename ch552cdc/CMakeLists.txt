cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE cmake/TC-sdcc-msc51.cmake)

project(main C ASM)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tool
)

add_compile_definitions(
    -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
    -DPRINTF_DISABLE_SUPPORT_FLOAT
    -DCLOCK_FREQ_24
    FREQ_SYS=24000000
)

file(GLOB_RECURSE MAIN_SRC "src/*.c" "include/*.c")
add_executable(${PROJECT_NAME} ${MAIN_SRC})

# 添加一个自定义命令，将.ihx文件转换为.hex文件
# 这个命令将在构建${PROJECT_NAME}时自动执行
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND packihx ${PROJECT_NAME}.ihx > ${PROJECT_SOURCE_DIR}/output/${PROJECT_NAME}.hex
    COMMENT "Generating HEX file from IHX file"
)