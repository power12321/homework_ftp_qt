cmake_minimum_required(VERSION 3.19)
project(Server LANGUAGES CXX)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 精简 Windows 头
add_definitions(-DWIN32_LEAN_AND_MEAN)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Server
    WIN32 MACOSX_BUNDLE
    main.cpp
    MainWindow.cpp
    MainWindow.h
    WinsockServer.h
    WinsockServer.cpp
)

target_link_libraries(Server
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# —— 在 Windows 下链接 Winsock2 ——
if(WIN32)
    target_link_libraries(Server
        PRIVATE
            ws2_32
    )
endif()

include(GNUInstallDirs)

install(TARGETS Server
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Server
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
