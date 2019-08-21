
#ifndef FFMPEGDEMO_FFMPEGCODE_H
#define FFMPEGDEMO_FFMPEGCODE_H
#include <jni.h>
#ifdef __cplusplus
extern "C" {
#endif

//编码信息
JNIEXPORT jstring JNICALL avcode_info(JNIEnv *, jobject);
//动态注册方法
JNIEXPORT jint JNICALL jni_register(JavaVM *vm, void *reserved, JNINativeMethod *, char *);
//视频解码
JNIEXPORT void JNICALL decode(JNIEnv *, jobject , jstring, jstring);
//雷博士的视频解码
JNIEXPORT jint JNICALL decode_lbs(JNIEnv *, jobject , jstring, jstring);

#ifdef __cplusplus
};
#endif

#endif //FFMPEGDEMO_FFMPEGCODE_H

