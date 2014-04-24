project(fractal-viewer)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

add_executable(fractal-viewer main.cpp)

target_link_libraries(fractal-viewer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

install(TARGETS fractal-viewer RUNTIME DESTINATION bin)

add_definitions(-fPIC)