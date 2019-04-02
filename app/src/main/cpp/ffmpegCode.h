//
// Created by Administrator on 2019/4/2.
//

#ifndef FFMPEGDEMO_FFMPEGCODE_H
#define FFMPEGDEMO_FFMPEGCODE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

//在这里定义头文件方法
JNIEXPORT jstring JNICALL avcodeinfo(JNIEnv *,jobject);

#ifdef __cplusplus
};
#endif

#endif //FFMPEGDEMO_FFMPEGCODE_H
