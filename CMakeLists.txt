cmake_minimum_required(VERSION 3.16)

project(forting VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appforting
    main.cpp
)

qt_add_qml_module(appforting
    URI forting
    QML_FILES
    MainWindow.qml
        SOURCES fortingcore.h fortingcore.cpp
        SOURCES parser.h
        SOURCES parser.cpp
        SOURCES fortstruct.h
        SOURCES
        SOURCES airuleparser.cpp
        SOURCES airuleparser.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appforting PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appforting
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appforting
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appforting
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
