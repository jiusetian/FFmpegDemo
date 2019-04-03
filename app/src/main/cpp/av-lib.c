/**
 * jni层向外暴露的接口
 */
#include <jni.h>
#include <android/log.h>
#include "ffmpegCode.h"
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"

JNIEXPORT jstring JNICALL avcodeInfo(JNIEnv *env, jobject ob) {
    return avcode_info(env,ob);
}


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    //要注册的方法集合，这里利用了映射形式
    const JNINativeMethod method[] = {
            {"avcodeInfo", "()Ljava/lang/String;", (void *) avcodeInfo}
    };

    //声明native方法的java文件
    jclass jClassName = (*env)->FindClass(env, "com/ffmpegdemo/AvUtil");

    jint ret = (*env)->RegisterNatives(env, jClassName, method,
                                       sizeof(method) / sizeof(JNINativeMethod));

    if (ret != JNI_OK) {
        __android_log_print(ANDROID_LOG_DEBUG, "JNITag", "jni_register Error");
        return -1;
    }
    __android_log_print(ANDROID_LOG_DEBUG, "JNITag", "注册成功");
    return JNI_VERSION_1_6;
}
