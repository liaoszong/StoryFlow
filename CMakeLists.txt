cmake_minimum_required(VERSION 3.16)

project(StoryFlow VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Quick Network QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)

set(CMAKE_AUTORCC ON)
qt_add_executable(appStoryFlow
    src/main.cpp
)

target_include_directories(appStoryFlow PUBLIC ${CMAKE_SOURCE_DIR}/src)

qt_add_qml_module(appStoryFlow
    URI StoryFlow
    VERSION 1.0
    QML_FILES
        src/ui/Main.qml
        src/ui/Components/LeftPage.qml
        src/ui/Components/RightPage.qml
        src/ui/Components/TopPage.qml
        src/ui/Pages/CreatePage.qml
        src/ui/Pages/StoryboardPage.qml
        src/ui/Pages/AssetsPage.qml
        RESOURCES src/ui/res.qrc
        QML_FILES src/ui/Pages/PreviewPage.qml
        QML_FILES src/ui/Pages/ShotDetailPage.qml
        SOURCES src/backend.h src/backend.cpp
        SOURCES
        SOURCES src/core/net/apiservice.h src/core/net/apiservice.cpp
        SOURCES src/core/viewmodel/storyviewmodel.h src/core/viewmodel/storyviewmodel.cpp
        SOURCES src/core/viewmodel/assetsviewmodel.h src/core/viewmodel/assetsviewmodel.cpp
        SOURCES src/core/player/timelinerenderer.h src/core/player/timelinerenderer.cpp
        # src/ui/Pages/ShotDetail.qml
        # src/ui/Pages/PreviewPage.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStoryFlow PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStoryFlow
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStoryFlow
    PRIVATE Qt6::Quick
    Qt6::Network
    Qt6::QuickControls2
)
target_link_libraries(appStoryFlow PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS appStoryFlow
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
