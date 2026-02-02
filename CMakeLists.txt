cmake_minimum_required(VERSION 3.16)

project(AmberParticleSSH LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -------------------------------------------------------------------------
# GTX 1080 Ti Optimization Flags (Pascal Architecture)
# -------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
endif()

# -------------------------------------------------------------------------
# Dependencies
# -------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Network OpenGLWidgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH2 REQUIRED libssh2)

# -------------------------------------------------------------------------
# Sources
# -------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/renderer/TerminalWidget.cpp
    src/renderer/ShaderManager.cpp
    src/particles/ParticleSystem.cpp
    src/terminal/SshClient.cpp
    src/terminal/PortForwarder.cpp
    src/terminal/TerminalModel.cpp
    src/ui/ConnectionDialog.cpp
    src/ui/MainWindow.cpp
    src/ui/TerminalTab.cpp
    src/ui/GraphicsSettingsDialog.cpp
    
    # libvterm
    src/vendor/libvterm/src/encoding.c
    src/vendor/libvterm/src/keyboard.c
    src/vendor/libvterm/src/mouse.c
    src/vendor/libvterm/src/parser.c
    src/vendor/libvterm/src/pen.c
    src/vendor/libvterm/src/screen.c
    src/vendor/libvterm/src/state.c
    src/vendor/libvterm/src/unicode.c
    src/vendor/libvterm/src/vterm.c
)

# -------------------------------------------------------------------------
# Executable
# -------------------------------------------------------------------------
add_executable(AmberParticleSSH ${SOURCES})

target_link_libraries(AmberParticleSSH PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Network
    Qt6::OpenGLWidgets
    ${LIBSSH2_LIBRARIES}
)

target_include_directories(AmberParticleSSH PRIVATE
    src
    ${LIBSSH2_INCLUDE_DIRS}
    src/vendor/libvterm/include
    src/vendor/libvterm/src
)

# Copy shaders to build directory
add_custom_command(TARGET AmberParticleSSH POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:AmberParticleSSH>/shaders
    COMMENT "Copying shaders to build directory"
)

# Enable shader resources later if we use Qt Resources, 
# for now we might load from filesystem or embed.
# qt_add_resources(...)

# -------------------------------------------------------------------------
# Custom Install or Run targets (Optional)
# -------------------------------------------------------------------------
