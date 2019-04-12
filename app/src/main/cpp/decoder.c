/**
 * 视频解码器
 */
#include <jni.h>
#include <android/log.h>
#include "ffmpegCode.h"
//解码
#include "libavcodec/avcodec.h"
//封装格式
#include "libavformat/avformat.h"
//视频像素格式转换
#include "libswscale/swscale.h"

#include <libavutil/imgutils.h>
//定义log的宏
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, "解码器", FORMAT, ##__VA_ARGS__)
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR, "解码器", FORMAT, ##__VA_ARGS__)

/**
 * 解码方法
 * @param env
 * @param type
 * @param input_jstr
 * @param ouput_jstr
 */
JNIEXPORT void JNICALL decode(JNIEnv *env, jobject ob, jstring input_jstr, jstring ouput_jstr) {

    //获取输入和输出文件的路径
    const char *input_str = (*env)->GetStringUTFChars(env, input_jstr, NULL);
    const char *ouput_str = (*env)->GetStringUTFChars(env, ouput_jstr, NULL);

    //注册所有组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // 打开输入视频文件，成功返回0，第三个参数为NULL，表示自动检测文件格式
    int code = avformat_open_input(&pFormatCtx, input_str, NULL, NULL);
    if (code != 0) {
        LOGE("%s%d", "打开输入视频文件失败：", code);
        return;
    }

    //获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频文件信息失败");
        return;
    }

    //查找视频流所在位置，遍历所有类型的流，找到视频流的所在位置
    int video_stream_index = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        //新api写法
//        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            video_stream_index = i;
//            break;
//        }
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    //AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codecpar;
    //编解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
    //查找解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "查找解码器失败");
        return;
    }

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("%s", "打开解码器失败");
        return;
    }

    //保存编码的数据包，分配内存
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));

    //像素格式（解码数据），存储一帧解码后像素数据
    AVFrame *pFrame = av_frame_alloc(); //用来存储解码得到的数据
    AVFrame *pYuvFrame = av_frame_alloc(); //用来存储解码后目标格式的数据
    //解码后的保存路径
    FILE *fp_yuv = fopen(ouput_str, "wb");

    //只有指定了avframe的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    //初始化缓存区，将pYuvFrame结构内指针指向out_buffer的数据，到这里才真正指定了pYuvFrame的像素格式和画面大小
    avpicture_fill((AVPicture *) pYuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width,
                   pCodecCtx->height);

    //srcW：源图像的宽
    //srcH：源图像的高
    //srcFormat：源图像的像素格式
    //dstW：目标图像的宽
    //dstH：目标图像的高
    //dstFormat：目标图像的像素格式
    //flags：设定图像拉伸使用的算法
    struct SwsContext *pSwsCtx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
            SWS_BICUBLIN, NULL, NULL, NULL
    );

    int got_frame, len, frameCount = 0;
    //从输入文件中一帧一帧读取压缩的视频数据AVpacket，
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        //如果读取的当前帧数据是我们想要的视频流
        if (pPacket->stream_index == video_stream_index) {
            //解码一帧压缩数据，第二个参数pFrame用来存储解码得到的数据，第三个参数为0时表示解码完成，第四个参数pPacket代表要解码的数据包
            len = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, pPacket);
            if (len < 0) {
                LOGE("%s", "解码失败");
                return;
            }
            //将源pFrame数据转换为pYuvFrame数据
            sws_scale(
                    pSwsCtx,
                    pFrame->data, pFrame->linesize, 0, pFrame->height,
                    pYuvFrame->data, pYuvFrame->linesize
            );
            //非0表示正在解码，0表示解码结束
            if (got_frame) {
                //图像宽高的乘积就是视频的总像素，而一个像素包含一个y，u对应1/4个y，v对应1/4个y
                int yuv_size = pCodecCtx->width * pCodecCtx->height;
                //写入y数据
                fwrite(pYuvFrame->data[0], 1, yuv_size, fp_yuv);
                //写入u数据
                fwrite(pYuvFrame->data[1], 1, yuv_size / 4, fp_yuv);
                //写入v数据
                fwrite(pYuvFrame->data[2], 1, yuv_size / 4, fp_yuv);
                LOGI("解码第%d帧", frameCount++);
            }
            av_free_packet(pPacket);
        }
    }

    //释放资源
    fclose(fp_yuv);
    av_frame_free(&pFrame);
    av_frame_free(&pYuvFrame);
    avcodec_free_context(&pCodecCtx);
    avformat_free_context(pFormatCtx);

    (*env)->ReleaseStringUTFChars(env, input_jstr, input_str);
    (*env)->ReleaseStringUTFChars(env, ouput_jstr, ouput_str);

}





















