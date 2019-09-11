//
// Created by public on 2019/9/11.
//

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
};
#endif
#endif


int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          0x0020))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                    NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

/*
av_register_all()：注册FFmpeg所有编解码器。
avformat_alloc_output_context2()：初始化输出码流的AVFormatContext。
avio_open()：打开输出文件。
av_new_stream()：创建输出码流的AVStream。
avcodec_find_encoder()：查找编码器。
avcodec_open2()：打开编码器。
avformat_write_header()：写文件头（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）。
avcodec_encode_video2()：编码一帧视频。即将AVFrame（存储YUV像素数据）编码为AVPacket（存储H.264等格式的码流数据）。
av_write_frame()：将编码后的视频码流写入文件。
flush_encoder()：输入的像素数据读取完成后调用此函数。用于输出编码器中剩余的AVPacket。
av_write_trailer()：写文件尾（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）。
 */
int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx;
    AVOutputFormat *fmt;
    AVStream *video_st;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVPacket pkt;
    uint8_t *picture_buf;
    AVFrame *pFrame;
    int picture_size;
    int y_size;
    int framecnt = 0;
    //FILE *in_file = fopen("src01_480x272.yuv", "rb");	//Input raw YUV data
    FILE *in_file = fopen("../ds_480x272.yuv", "rb");   //Input raw YUV data
    //输入视频的宽高
    int in_w = 480, in_h = 272;                              //Input data's width and height

    int framenum = 100;                                   //Frames to encode
    //const char* out_file = "src01.h264";              //Output Filepath
    //const char* out_file = "src01.ts";
    //const char* out_file = "src01.hevc";
    //输出文件的格式
    const char *out_file = "ds.h264";
    //注册FFmpeg所有编解码器
    av_register_all();
    //M初始化输出码流的AVFormatContext
    pFormatCtx = avformat_alloc_context();
    //猜测输出码流的格式
    fmt = av_guess_format(NULL, out_file, NULL);
    //赋值
    pFormatCtx->oformat = fmt;

    //Method 2. 会自己猜测输出格式
    //avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
    //fmt = pFormatCtx->oformat;


    //Open output URL，打开输出文件
    if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Failed to open output file! \n");
        return -1;
    }
    //在 AVFormatContext 中创建 Stream 通道，不指定编码器，必须在写头文件之前调用，这里只是创建内存
    video_st = avformat_new_stream(pFormatCtx, 0);
    //video_st->time_base.num = 1;
    //video_st->time_base.den = 25;

    if (video_st == NULL) {
        return -1;
    }

    //Param that must set，设置编码的相关参数

    pCodecCtx = video_st->codec; /*AVCodecContext 相当于虚基类，需要用具体的编码器实现来给他赋值*/
    //pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
    pCodecCtx->codec_id = fmt->video_codec; //编码器ID
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO; //编码器类型
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // 像素格式
    //编码目标的视频帧大小，以像素为单位
    pCodecCtx->width = in_w; //视频宽
    pCodecCtx->height = in_h; //视频高
    //码率是指视频图像经过编码压缩后，在单位时间内的数据流量，视频中比特率计算公式：比特率 = 帧率 * 每帧数据大小
    //所以说视频的码率越大，帧率或者每帧数据就越大，然后视频就越清晰，这是一个需要权衡的数据
    pCodecCtx->bit_rate = 400000; //目标的码率，即采样的码率；显然，采样码率越大，视频大小越大
    pCodecCtx->gop_size = 250; // 图像组大小
    //非压缩数据，time_base为对应结构体AVCodecContext中的值，其中分母den为帧率，所以这个time_base代表一帧的时间
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //H264
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    //设置量化参数. 量化系数越小，视频越清晰（这个量化参数都是些数学算法，有兴趣可以了解一下），这里采用默认值。
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;

    //两个非B帧之间允许出现多少个B帧数
    //设置0表示不使用B帧
    //b 帧越多，图片越小
    //Optional Param，设置B帧最大值
    pCodecCtx->max_b_frames = 3;

    // Set Option
    AVDictionary *param = 0;
    //H.264
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        //编码速度选项：
        //第一个值：预备参数
        //key: preset
        //value: slow->慢
        //value: superfast->超快
        av_dict_set(param, "preset", "slow", 0);
        //延时选项：
        //key: tune->调优
        //value: zerolatency->零延迟
        av_dict_set(param, "tune", "zerolatency", 0);
        //av_dict_set(¶m, "profile", "main", 0);
    }
    //H.265
    if (pCodecCtx->codec_id == AV_CODEC_ID_H265) {
        av_dict_set(param, "preset", "ultrafast", 0);
        av_dict_set(param, "tune", "zero-latency", 0);
    }

    //Show some Information，打印流信息
    av_dump_format(pFormatCtx, 0, out_file, 1);
    //查找编码器
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        printf("Can not find encoder! \n");
        return -1;
    }

    //打开编码器
    if (avcodec_open2(pCodecCtx, pCodec, param) < 0) {
        printf("Failed to open encoder! \n");
        return -1;
    }

    //AVframe一般用来封装未压缩数据
    pFrame = av_frame_alloc();
    picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    picture_buf = (uint8_t *) av_malloc(picture_size);
    avpicture_fill((AVPicture *) pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width,
                   pCodecCtx->height);

    //Write File Header，写头文件
    avformat_write_header(pFormatCtx, NULL);

    av_new_packet(&pkt, picture_size);

    y_size = pCodecCtx->width * pCodecCtx->height;

    //循环读取framenum帧进行编码
    for (int i = 0; i < framenum; i++) {
        //Read raw YUV data，读取原始的YUV数据到picture_buf中
        if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0) {
            printf("Failed to read raw data! \n");
            return -1;
        } else if (feof(in_file)) {
            break;
        }
        pFrame->data[0] = picture_buf;              // Y
        pFrame->data[1] = picture_buf + y_size;      // U
        pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
        //PTS
        //pFrame->pts=i;因为这里的帧率是25，pts又是时间戳，比如当i=1的时候，代表第二帧的播放时间，这里计算得到1/25，单位是秒，即第二帧在1/25秒的时候播放
        pFrame->pts = i * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
        int got_picture = 0;
        //Encode 编码，解码得到的数据保存在pkt中
        int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
        if (ret < 0) {
            printf("Failed to encode! \n");
            return -1;
        }
        if (got_picture == 1) {
            printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
            framecnt++;
            pkt.stream_index = video_st->index;
            //将编码后的视频码流写入文件
            ret = av_write_frame(pFormatCtx, &pkt);
            av_free_packet(&pkt);
        }
    }

    //Flush Encoder
    //输入的像素数据读取完成后调用此函数。用于输出编码器中剩余的AVPacket。
    int ret = flush_encoder(pFormatCtx, 0);
    if (ret < 0) {
        printf("Flushing encoder failed\n");
        return -1;
    }

    //Write file trailer
    //写文件尾（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）
    av_write_trailer(pFormatCtx);

    //Clean
    if (video_st) {
        avcodec_close(video_st->codec);
        av_free(pFrame);
        av_free(picture_buf);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    fclose(in_file);

    return 0;
}