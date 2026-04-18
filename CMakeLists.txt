cmake_minimum_required(VERSION 3.16)
project(Monopoke VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── SFML（预编译包） ───────────────────────────────────
set(SFML_ROOT "C:/SFML/SFML-2.6.2")
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

# nlohmann/json 已直接下载到 src/json.hpp（无需 FetchContent）

# ── 源文件收集 ──────────────────────────────────────────
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# ── 可执行文件 ──────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics sfml-window sfml-system sfml-audio
)

# ── 复制资源文件到输出目录 ────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# ── 复制存档目录 ─────────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/saves
)

# ── Windows下复制 SFML DLL 到输出目录 ─────────────────────
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SFML_ROOT}/bin
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
