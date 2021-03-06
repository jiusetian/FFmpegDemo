
cmake_minimum_required(VERSION 3.4.1)

#集成ffmpeg的时候，不能同时有两个add_library
#add_library(
#        native-lib
#        SHARED
#        main/cpp/native-lib.cpp
#)
#add_library(
#        avcode-info
#        SHARED
#        main/cpp/avcode_info.c
#)
add_library(
        av-lib
        SHARED
        main/jni/main-lib.c
        main/jni/jni_util.c
        main/jni/processDecode/avcode_info.c
        main/jni/processDecode/decoder.c
        main/jni/processDecode/decode_l.c
        main/jni/avEntry/aac.c
        main/jni/avEntry/flv.c
        main/jni/avEntry/h264.c
        main/jni/avEntry/pcm.c
        main/jni/avEntry/yuv_rgb.c
        main/jni/processDecode/udp_rtp.c
        main/jni/Codec/yuv_2_h264.c
        main/jni/Codec/pcm_2_aac.c
)


find_library(
        log-lib
        log)

#target_link_libraries(
#        av-lib
#        ${log-lib}
#)

#下面是加载ffmpeg的库和头文件

#相当于指定一个变量，后面可以通过${var}来引用，这里我们指定ffmpeg的库文件路径
set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/main/ffmpeg/${ANDROID_ABI})

#添加库文件
#工具库（大部分库都需要这个库的支持）
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
#音频采样数据格式转换
add_library(swresample
        SHARED
        IMPORTED )

set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libswresample-3.so )

#视频像素格式转换
add_library(swscale
        SHARED
        IMPORTED )

set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libswscale-5.so )

#编解码（最重要的库）
add_library(avcodec
        SHARED
        IMPORTED )

set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavcodec-58.so )

#封装格式处理
add_library(avformat
        SHARED
        IMPORTED )

set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavformat-58.so )

#滤镜特效处理
add_library(avfilter
        SHARED
        IMPORTED )

set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/libavfilter-7.so )

#各种设备的输入输出
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
        #对外开放的接口
        av-lib
        #第三方库
        avutil
        swresample
        swscale
        avcodec
        avformat
        avfilter
        avdevice
        android
        ${log-lib})

