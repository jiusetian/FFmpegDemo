
cmake_minimum_required(VERSION 3.4.1)

#add_library(
#        native-lib
#        SHARED
#        main/cpp/native-lib.cpp)

add_library(
        av-lib
        SHARED
        main/cpp/av-lib.c
)

find_library(
        log-lib
        log)

#下面是加载ffmpeg的库和头文件

#相当于指定一个变量，后面可以通过${var}来引用，这里我们指定ffmpeg的库文件路径
set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/main/ffmpeg/${ANDROID_ABI})

#添加库文件
add_library(
        avutil
        SHARED
        IMPORTED
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavutil-56.so
)
#
add_library(swresample
        SHARED
        IMPORTED )

set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libswresample-3.so )
#
add_library(swscale
        SHARED
        IMPORTED )

set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libswscale-5.so )
#
add_library(avcodec
        SHARED
        IMPORTED )

set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavcodec-58.so )
#
add_library(avformat
        SHARED
        IMPORTED )

set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavformat-58.so )
#
add_library(avfilter
        SHARED
        IMPORTED )

set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavfilter-7.so )
#
add_library(avdevice
        SHARED
        IMPORTED )
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavdevice-58.so )

#指定ffmpeg头文件的位置
include_directories(main/ffmpeg/include)

#链接so库
target_link_libraries(
        av-lib
        avutil
        swresample
        swscale
        avcodec
        avformat
        avfilter
        avdevice
        android
        ${log-lib})

