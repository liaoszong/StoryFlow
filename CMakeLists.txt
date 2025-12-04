cmake_minimum_required(VERSION 3.16)

project(StoryFlow VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置UTF-8编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Core Network QuickControls2 Multimedia)

qt_standard_project_setup(REQUIRES 6.8)

# FFmpeg 路径
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg")
include_directories(${FFMPEG_DIR}/include)

# VideoCreator 模块源文件
set(VIDEOCREATOR_SOURCES
    src/videocreator/model/ConfigLoader.cpp
    src/videocreator/model/ConfigLoader.h
    src/videocreator/model/ProjectConfig.h
    src/videocreator/engine/RenderEngine.cpp
    src/videocreator/engine/RenderEngine.h
    src/videocreator/decoder/ImageDecoder.cpp
    src/videocreator/decoder/ImageDecoder.h
    src/videocreator/decoder/AudioDecoder.cpp
    src/videocreator/decoder/AudioDecoder.h
    src/videocreator/decoder/VideoDecoder.cpp
    src/videocreator/decoder/VideoDecoder.h
    src/videocreator/filter/EffectProcessor.cpp
    src/videocreator/filter/EffectProcessor.h
    src/videocreator/ffmpeg_utils/AvFrameWrapper.h
    src/videocreator/ffmpeg_utils/AvPacketWrapper.h
    src/videocreator/ffmpeg_utils/AvFormatContextWrapper.h
    src/videocreator/ffmpeg_utils/AvCodecContextWrapper.h
    src/videocreator/ffmpeg_utils/FFmpegHeaders.h
    src/videocreator/VideoCreatorAPI.cpp
    src/videocreator/VideoCreatorAPI.h
)

# VideoCreator 静态库
add_library(VideoCreatorCore STATIC ${VIDEOCREATOR_SOURCES})

target_include_directories(VideoCreatorCore PUBLIC
    ${CMAKE_SOURCE_DIR}/src/videocreator
    ${CMAKE_SOURCE_DIR}/src/videocreator/model
    ${CMAKE_SOURCE_DIR}/src/videocreator/ffmpeg_utils
    ${CMAKE_SOURCE_DIR}/src/videocreator/engine
    ${CMAKE_SOURCE_DIR}/src/videocreator/decoder
    ${CMAKE_SOURCE_DIR}/src/videocreator/filter
)

# 链接 FFmpeg 到 VideoCreatorCore
target_link_libraries(VideoCreatorCore PUBLIC
    Qt6::Core
    ${FFMPEG_DIR}/lib/avcodec.lib
    ${FFMPEG_DIR}/lib/avformat.lib
    ${FFMPEG_DIR}/lib/avutil.lib
    ${FFMPEG_DIR}/lib/swscale.lib
    ${FFMPEG_DIR}/lib/swresample.lib
    ${FFMPEG_DIR}/lib/avfilter.lib
)

# 主程序
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
        src/ui/Pages/PreviewPage.qml
        src/ui/Pages/ShotDetailPage.qml
    RESOURCES
        src/ui/res.qrc
    SOURCES
        src/core/net/apiservice.h src/core/net/apiservice.cpp
        src/core/viewmodel/storyviewmodel.h src/core/viewmodel/storyviewmodel.cpp
        src/core/viewmodel/assetsviewmodel.h src/core/viewmodel/assetsviewmodel.cpp
        src/core/player/timelinerenderer.h src/core/player/timelinerenderer.cpp
        src/core/player/engine/storyengine.h src/core/player/engine/storyengine.cpp
        src/core/video/videogenerator.h src/core/video/videogenerator.cpp
)

set_target_properties(appStoryFlow PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 链接库
target_link_libraries(appStoryFlow PRIVATE
    Qt6::Quick
    Qt6::Core
    Qt6::Network
    Qt6::QuickControls2
    Qt6::Multimedia
    VideoCreatorCore
)

include(GNUInstallDirs)
install(TARGETS appStoryFlow
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 复制 FFmpeg DLL 到输出目录（Windows）
if(WIN32)
    if(EXISTS "${FFMPEG_DIR}/bin/")
        file(GLOB FFMPEG_DLLS "${FFMPEG_DIR}/bin/*.dll")
        if(FFMPEG_DLLS)
            foreach(DLL_FILE ${FFMPEG_DLLS})
                add_custom_command(TARGET appStoryFlow POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${DLL_FILE}
                    $<TARGET_FILE_DIR:appStoryFlow>
                )
            endforeach()
            message(STATUS "FFmpeg DLLs will be copied to output directory")
        endif()
    endif()
endif()
