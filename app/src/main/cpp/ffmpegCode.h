
#ifndef FFMPEGDEMO_FFMPEGCODE_H
#define FFMPEGDEMO_FFMPEGCODE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif


extern JNIEXPORT jstring JNICALL avcodemsg(JNIEnv *,jobject);

JNIEXPORT jint JNICALL decode();

#ifdef __cplusplus
};
#endif

#endif //FFMPEGDEMO_FFMPEGCODE_H

