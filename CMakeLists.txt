cmake_minimum_required(VERSION 3.13)

project(smbus C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(PROJECT_LIB ${PROJECT_NAME})
set(PROJECT_CMD ${PROJECT_NAME}-commander)

# SYSROOT_ENV BEGIN 
set(RASPBIAN_DIR "$ENV{HOME}/raspbian")
set(RASPBIAN_TARGET_ROOT "${RASPBIAN_DIR}/sshfs")
set(CMAKE_SYSROOT "${RASPBIAN_DIR}/cross-compile/chroots/rpi-bullseye-armhf")
set(RASPBIAN_INSTALL_PREFIX "${RASPBIAN_TARGET_ROOT}/home/pi")
# SYSROOT_ENV BEGIN

# Library part
add_library(${PROJECT_LIB} STATIC
    lib/smbus.c
    lib/smbus_pec.c
)
target_link_libraries(${PROJECT_LIB}
    i2c
)
target_include_directories(${PROJECT_LIB} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
    $<INSTALL_INTERFACE:${PROJECT_ROOT}include/smbus>
)
target_compile_options(${PROJECT_LIB} PRIVATE -Wall)


# Commander part
add_executable(${PROJECT_CMD}
    cmd/main.c
    cmd/commands.h
)
target_link_libraries(${PROJECT_CMD}
    ${PROJECT_LIB}
)
target_include_directories(${PROJECT_LIB} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_ROOT}/include>
)
target_compile_options(${PROJECT_LIB} PRIVATE -Wall)


install(
    TARGETS ${PROJECT_CMD}
    RUNTIME
    DESTINATION "${RASPBIAN_INSTALL_PREFIX}/${PROJECT_NAME}/"
)