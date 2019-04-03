
#ifndef FFMPEGDEMO_FFMPEGCODE_H
#define FFMPEGDEMO_FFMPEGCODE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

//编码信息
extern JNIEXPORT jstring JNICALL avcode_info(JNIEnv *,jobject);
//

JNIEXPORT jint JNICALL decode();

#ifdef __cplusplus
};
#endif

#endif //FFMPEGDEMO_FFMPEGCODE_H

