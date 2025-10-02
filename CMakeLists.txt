cmake_minimum_required(VERSION 3.16)

if (NOT BACKEND)
    set(BACKEND SDL CACHE STRING "PSX system backend" FORCE)
endif()
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build Type" FORCE)
endif()

project(PSX-Emulator
    VERSION 1.0
    DESCRIPTION "Simple Playstation 1 Emulator"
    LANGUAGES C)
########################################################################
# list of all packages
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
########################################################################

########################################################################
add_library(release-flags INTERFACE)
target_compile_options(release-flags
    INTERFACE
    -O3
    -Wall
    -Wextra
    -Wfloat-equal
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wstrict-prototypes
    -Wstrict-overflow
    -Wno-unused-variable
    -Wno-unused-parameter
    -march=native)
add_library(debug-flags INTERFACE)
target_compile_options(debug-flags
    INTERFACE
    -g
    -Wall
    -Wextra
    -Wfloat-equal
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wstrict-prototypes
    -Wstrict-overflow
    -Wno-unused-variable
    -Wno-unused-parameter
    -fsanitize=address
    -march=native)
target_compile_definitions(debug-flags
    INTERFACE
    -D TRACE_TTY
    $<$<BOOL:${GDBSTUB}>:GDBSTUB>
    -D ENABLE_CPU_TRACE
    -D ENABLE_GPU_TRACE
    -D ENABLE_SPU_TRACE
    -D ENABLE_DMA_TRACE
    -D ENABLE_BIOS_TRACE
    -D ENABLE_MEMORY_TRACE
    -D ENABLE_MEMORY_DEVICE_TRACE
    -D ENABLE_TIMERS_TRACE
    -D ENABLE_INTERRUPTS_TRACE
    -D ENABLE_SYS_TRACE)
target_link_options(debug-flags
    INTERFACE
    -fsanitize=address)
########################################################################

########################################################################
#### core
add_library(psx-core
    ${CMAKE_CURRENT_SOURCE_DIR}/cdrom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cp0.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gte.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/spu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/trace.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cdrom.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cp0.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gte.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dma.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fifo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fileformats.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd_avx512.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd_avx256.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simd_sse128.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/system.h
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trace.h)
target_link_libraries(psx-core
    PUBLIC
    PRIVATE
    $<$<CONFIG:RELEASE>: release-flags>
    $<$<CONFIG:DEBUG>: debug-flags>)

#### debugger
add_library(psx-debugger
    ${CMAKE_CURRENT_SOURCE_DIR}/main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stub.h)
target_link_libraries(psx-debugger
    PUBLIC
    PRIVATE
    debug-flags)

#### SDL
add_library(psx-sdl
    ${CMAKE_CURRENT_SOURCE_DIR}/system_sdl.c)
target_include_directories(psx-sdl
    PUBLIC
    PRIVATE
    ${SDL_INCLUDE_DIRS})
target_link_libraries(psx-sdl
    PUBLIC
    PRIVATE
    $<$<CONFIG:RELEASE>: release-flags>
    $<$<CONFIG:DEBUG>: debug-flags>)

### Vulkan
add_library(psx-vulkan
    ${CMAKE_CURRENT_SOURCE_DIR}/system_vulkan.c)
target_link_libraries(psx-vulkan
    PUBLIC
    PRIVATE
    $<$<CONFIG:RELEASE>: release-flags>
    $<$<CONFIG:DEBUG>: debug-flags>)
########################################################################

############################# TARGET ##################################
add_executable(emulator
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_include_directories(emulator
    PUBLIC
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(emulator
    PUBLIC
    PRIVATE
    psx-core
    psx-sdl
    psx-vulkan
    $<$<CONFIG:DEBUG>:psx-debugger>

    Threads::Threads
    ${SDL2_LIBRARIES}

    $<$<CONFIG:RELEASE>: release-flags>
    $<$<CONFIG:DEBUG>: debug-flags>)
########################################################################

############################ COMMANDS ##################################
add_custom_target(run
    COMMAND emulator -b BIOS.bin -g GAME.bin
    VERBATIM)
add_custom_target(run-debug
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND gdb -iex "set auto-load safe-path ${CMAKE_CURRENT_SOURCE_DIR}" --args emulator -b BIOS.bin -g GAME.bin
    VERBATIM)
add_custom_target(perf-record-general
    COMMAND perf record ./emulator -b BIOS.bin -g GAME.bin
    VERBATIM)
add_custom_target(perf-record-cache
    COMMAND perf record -e cache-references,cache-misses ./emulator -b BIOS.bin -g GAME.bin
    VERBATIM)
