
#include <stdio.h>
#include <time.h>
#include "../ffmpeg_code.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
#include "libavutil/imgutils.h"

#ifdef ANDROID

#include <jni.h>
#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("(>_<) " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("(^_^) " format "\n", ##__VA_ARGS__)
#endif


//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}
/**
 * 雷霄烨博士的解码方法
 * @param env
 * @param obj
 * @param input_jstr
 * @param output_jstr
 * @return
 */
JNIEXPORT jint JNICALL
decode_lbs(JNIEnv *env, jobject obj, jstring input_jstr, jstring output_jstr) {
    //封装格式上下文结构体，保存视频文件封装格式相关信息
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    //编码器上下文结构体，保存视音频编解码相关信息
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;
    FILE *fp_yuv;
    int frame_cnt;
    clock_t time_start, time_finish;
    double time_duration = 0.0;

    char input_str[500] = {0};
    char output_str[500] = {0};
    char info[1000] = {0};
    //将视频文件输入输出路径写入内存中
    sprintf(input_str, "%s", (*env)->GetStringUTFChars(env, input_jstr, NULL));
    sprintf(output_str, "%s", (*env)->GetStringUTFChars(env, output_jstr, NULL));

    //FFmpeg av_log() callback
    av_log_set_callback(custom_log);

    //新版本可以不用这个方法
    //av_register_all();

    //打开网络流的话，前面要加上函数avformat_network_init()
    avformat_network_init();
    //封装格式上下文
    pFormatCtx = avformat_alloc_context();

    //打开输入视频文件，成功返回0，第三个参数为NULL，表示自动检测文件格式
    if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0) {
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    //获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    //查找视频流所在位置
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }

    if (videoindex == -1) {
        LOGE("Couldn't find a video stream.\n");
        return -1;
    }
    //初始化编码器结构上下文
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    //找到对应的编码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("Couldn't find Codec.\n");
        return -1;
    }
    //打开编码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Couldn't open codec.\n");
        return -1;
    }
    //存储一帧解码后的像素数据
    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    //缓冲区分配内存
    out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    //指定pFrameYUV的像素格式和大小
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    //保存编码的数据包
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //srcW：源图像的宽
    //srcH：源图像的高
    //srcFormat：源图像的像素格式
    //dstW：目标图像的宽
    //dstH：目标图像的高
    //dstFormat：目标图像的像素格式
    //flags：设定图像拉伸使用的算法
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL, NULL, NULL);


    sprintf(info, "[Input     ]%s\n", input_str);
    sprintf(info, "%s[Output    ]%s\n", info, output_str);
    sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);

    //打开存储编码后的格式文件
    fp_yuv = fopen(output_str, "wb+");

    if (fp_yuv == NULL) {
        printf("Cannot open output file.\n");
        return -1;
    }

    frame_cnt = 0;
    time_start = clock();

    //从输入文件中一帧一帧读取视频数据到packet中
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //如果是视频流
        if (packet->stream_index == videoindex) {
            //解码一帧压缩数据，第二个参数pFrame用来存储解码得到的数据，第三个参数为0时表示解码完成，第四个参数pPacket代表要解码的数据包
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);

            if (ret < 0) {
                LOGE("Decode Error.\n");
                return -1;
            }
            //非0表示解码未结束，0表示解码已结束
            if (got_picture) {

                //将源pFrame数据转换为pYuvFrame数据
                sws_scale(img_convert_ctx,
                          (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                          pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                //图像宽高的乘积就是视频的总像素，而一个像素包含一个y，u对应1/4个y，v对应1/4个y
                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V

                //Output info
                char pictype_str[10] = {0};
                switch (pFrame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        sprintf(pictype_str, "I");
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pictype_str, "P");
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pictype_str, "B");
                        break;
                    default:
                        sprintf(pictype_str, "Other");
                        break;
                }
                LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
                frame_cnt++;
            }
        }
        av_packet_unref(packet);
        // av_free_packet(packet);
    }

    //如果在编码器中还有frame，则将其刷出
    //flush decoder
    //FIX: Flush Frames remained in Codec
    while (1) {
        LOGI("%s", "执行了后面的while循环01");
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        //0表示解码结束
        if (!got_picture)
            break;
        LOGI("%s", "执行了后面的while循环02");
        sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);

        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
        //Output info
        char pictype_str[10] = {0};
        switch (pFrame->pict_type) {
            case AV_PICTURE_TYPE_I:
                sprintf(pictype_str, "I");
                break;
            case AV_PICTURE_TYPE_P:
                sprintf(pictype_str, "P");
                break;
            case AV_PICTURE_TYPE_B:
                sprintf(pictype_str, "B");
                break;
            default:
                sprintf(pictype_str, "Other");
                break;
        }
        LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
        frame_cnt++;
    }
    time_finish = clock();
    time_duration = (double) (time_finish - time_start);

    sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
    sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
    LOGI("%s", "code finish：解码结束");
    return 0;
}

