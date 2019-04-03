#include <jni.h>
#include <android/log.h>

#include "ffmpegCode.h"



//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"


JNIEXPORT jstring JNICALL avcodeinfo(JNIEnv *env, jobject ob) {

    return avcodemsg(env,ob);
//    char info[40000] = {0};
//    //新版本已经不需要这个方法了
//    //av_register_all();
//
//    AVCodec *c_temp = av_codec_next(NULL);
//
//    while (c_temp != NULL) {
//        if (c_temp->decode != NULL) {
//            sprintf(info, "%sdecode:", info);
//        } else {
//            sprintf(info, "%sencode:", info);
//        }
//
//        switch (c_temp->type) {
//            case AVMEDIA_TYPE_VIDEO:
//                //格式化输出到 str 所指向的字符串
//                sprintf(info, "%s(video):", info);
//                break;
//            case AVMEDIA_TYPE_AUDIO:
//                sprintf(info, "%s(audio):", info);
//                break;
//            default:
//                sprintf(info, "%s(other):", info);
//                break;
//        }
//        sprintf(info, "%s[%10s]\n", info, c_temp->name);
//        c_temp = c_temp->next;
//    }
//    return (*env)->NewStringUTF(env, info);
}


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {

    JNIEnv *env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    //要注册的方法集合，这里利用了映射形式
    const JNINativeMethod method[] = {
            {"avcodeInfo", "()Ljava/lang/String;", (void *) avcodeinfo}
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
