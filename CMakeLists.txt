cmake_minimum_required(VERSION 3.16)

project(indexbourne VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(QuaZip-Qt6)
else()
    find_package(QuaZip-Qt5)
endif()

set(PROJECT_SOURCES
        src/main.cpp
        resources.qrc
        src/mainwindow.cpp src/mainwindow.h src/mainwindow.ui
        src/project.h src/project.cpp src/projectio.cpp
        src/canvas.h src/canvas.cpp
        src/palette_panel.cpp src/palette_panel.h src/palette_panel.ui
        src/tool_panel.h src/tool_panel.cpp src/tool_panel.ui
        src/newprojectdialog.h src/newprojectdialog.cpp src/newprojectdialog.ui
        src/resizedialog.h src/resizedialog.cpp src/resizedialog.ui
        src/layer_panel.h src/layer_panel.cpp src/layer_panel.ui
        src/exportdialog.h src/exportdialog.cpp src/exportdialog.ui
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(indexbourne
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET indexbourne APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(indexbourne SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(indexbourne
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(indexbourne PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(indexbourne PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
target_link_libraries(indexbourne PRIVATE QuaZip::QuaZip)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.indexbourne)
endif()
set_target_properties(indexbourne PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS indexbourne
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(indexbourne)
endif()
