cmake_minimum_required(VERSION 3.16)
project(command_center LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(command_center
    src/main.cpp
)

target_include_directories(command_center PRIVATE
    ${GTK3_INCLUDE_DIRS}
)

target_link_libraries(command_center PRIVATE
    ${GTK3_LIBRARIES}
)

add_definitions(
    ${GTK3_CFLAGS_OTHER}
)

# ───────────────────────────────────────────────
# Install rules (user-local or system-wide)
# ───────────────────────────────────────────────

install(TARGETS command_center
    RUNTIME DESTINATION bin
)

install(FILES data/command-center.desktop
    DESTINATION share/applications
)

