#include <jni.h>
#include <android/log.h>
#include "ffmpegCode.h"

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"

JNIEXPORT jstring JNICALL avcodemsg(JNIEnv *env, jobject ob) {

    char info[40000] = {0};
    //新版本已经不需要这个方法了
    //av_register_all();

    AVCodec *c_temp = av_codec_next(NULL);

    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%sdecode:", info);
        } else {
            sprintf(info, "%sencode:", info);
        }

        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                //格式化输出到 str 所指向的字符串
                sprintf(info, "%s(video):", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s(audio):", info);
                break;
            default:
                sprintf(info, "%s(other):", info);
                break;
        }
        sprintf(info, "%s[%10s]\n", info, c_temp->name);
        c_temp = c_temp->next;
    }
    return (*env)->NewStringUTF(env, info);
}