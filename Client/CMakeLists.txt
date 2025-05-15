cmake_minimum_required(VERSION 3.19)
project(Client LANGUAGES CXX)

# 如果你还没设置 C++ 标准，可以加上：
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 精简 Windows 头文件（可选）
add_definitions(-DWIN32_LEAN_AND_MEAN)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Client
    WIN32 MACOSX_BUNDLE
    main.cpp
    MainWindow.cpp
    MainWindow.h
    WinsockClient.h
    WinsockClient.cpp
)

target_link_libraries(Client
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# —— 新增 Winsock2 链接 ——
if(WIN32)
    target_link_libraries(Client
        PRIVATE
            ws2_32
    )
endif()

include(GNUInstallDirs)

install(TARGETS Client
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Client
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
