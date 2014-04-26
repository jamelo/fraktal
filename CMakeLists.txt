project(fractal-viewer)

set(CMAKE_AUTOMOC ON)

find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

kde4_add_executable(fractal-viewer
    main.cpp
    MainWindow.cpp
    ColorScheme.cpp
    BackgroundWorker.cpp
)

SET(CMAKE_CXX_FLAGS "-std=c++11")

target_link_libraries(fractal-viewer
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

install(TARGETS fractal-viewer DESTINATION bin)
install(FILES fractal-viewerui.rc DESTINATION  ${DATA_INSTALL_DIR}/fractal-viewer)

add_definitions(-fPIC)
