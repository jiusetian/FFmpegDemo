#include <jni.h>
#include <android/log.h>
#include "ffmpeg_code.h"


//动态注册方法
JNIEXPORT jint JNICALL jni_register(JavaVM *vm, void *reserved,JNINativeMethod *method, char *str)
{
    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    //声明native方法的java文件
    jclass jClassName = (*env)->FindClass(env, str);

    jint ret = (*env)->RegisterNatives(env, jClassName, method,
                                       sizeof(method) / sizeof(JNINativeMethod));

    if (ret != JNI_OK) {
        __android_log_print(ANDROID_LOG_DEBUG, "JNITag", "jni_register Error");
        return -1;
    }
    __android_log_print(ANDROID_LOG_DEBUG, "JNITag", "注册成功");
    return JNI_VERSION_1_6;
}