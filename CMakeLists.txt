cmake_minimum_required(VERSION 3.21)
project(MaxWordle)

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(
        MaxWordle
        main.cpp
        Wordle.cpp Wordle.h
        ./imgui/imgui.h
        ./imgui/imstb_truetype.h
        ./imgui/imstb_textedit.h
        ./imgui/imstb_rectpack.h
        ./imgui/imgui_internal.h
        ./imgui/imconfig.h
        ./imgui/imgui.cpp
        ./imgui/imgui_demo.cpp
        ./imgui/imgui_draw.cpp
        ./imgui/imgui_tables.cpp
        ./imgui/imgui_widgets.cpp
        ./imgui/backends/imgui_impl_glfw.h
        ./imgui/backends/imgui_impl_opengl3.h
        ./imgui/backends/imgui_impl_glfw.cpp
        ./imgui/backends/imgui_impl_opengl3.cpp
        ./imgui/backends/imgui_impl_opengl3_loader.h
)

target_link_libraries(MaxWordle libglfw.3.3.dylib)
target_link_libraries(MaxWordle "-framework OpenGL")