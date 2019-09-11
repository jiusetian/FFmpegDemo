//
// Created by public on 2019/9/11.
//

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif
#endif


int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index){
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
        ret = avcodec_encode_audio2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                     NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame){
            ret=0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

int main(int argc, char* argv[])
{
    AVFormatContext* pFormatCtx;
    AVOutputFormat* fmt;
    AVStream* audio_st;
    AVCodecContext* pCodecCtx;
    AVCodec* pCodec;

    uint8_t* frame_buf;
    AVFrame* pFrame;
    AVPacket pkt;

    int got_frame=0;
    int ret=0;
    int size=0;

    FILE *in_file=NULL;	                        //Raw PCM data
    int framenum=1000;                          //Audio frame number
    const char* out_file = "tdjm.aac";          //Output URL
    int i;
    //打开pcm文件
    in_file= fopen("tdjm.pcm", "rb");

    av_register_all();

    //Method 1.分配封装格式上下文
    pFormatCtx = avformat_alloc_context();
    fmt = av_guess_format(NULL, out_file, NULL);
    pFormatCtx->oformat = fmt; //编码格式


    //Method 2.
    //avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
    //fmt = pFormatCtx->oformat;

    //Open output URL 打开输入输出文件流
    if (avio_open(&pFormatCtx->pb,out_file, AVIO_FLAG_READ_WRITE) < 0){
        printf("Failed to open output file!\n");
        return -1;
    }

    audio_st = avformat_new_stream(pFormatCtx, 0);
    if (audio_st==NULL){
        return -1;
    }
    //相关参数设置
    pCodecCtx = audio_st->codec;
    pCodecCtx->codec_id = fmt->audio_codec; //编码器ID
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO; //编码器类型：音频

    //AV_SAMPLE_FMT_S16为非平面格式，其中数字16代表一个采样点用16bit内存保存，假设现在有2个通道channel1, channel2
    //AV_SAMPLE_FMT_S16在内存的格式就为: c1, c2, c1, c2, c1, c2, .... 交错保存
    //AV_SAMPLE_FMT_S16P在内存的格式为: c1, c1, c1,... c2, c2, c2,... 分开平铺
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16; //音频采样格式

    pCodecCtx->sample_rate= 44100; //采样率
    pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO; //双声道
    pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout); //声道数
    pCodecCtx->bit_rate = 64000; //比特率

    //Show some information
    av_dump_format(pFormatCtx, 0, out_file, 1);

    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec){
        printf("Can not find encoder!\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0){
        printf("Failed to open encoder!\n");
        return -1;
    }
    //分配音频帧内存
    pFrame = av_frame_alloc();
    pFrame->nb_samples= pCodecCtx->frame_size; //一帧有多少采样数
    pFrame->format= pCodecCtx->sample_fmt;
    //帧缓存空间的大小
    size = av_samples_get_buffer_size(NULL, pCodecCtx->channels,pCodecCtx->frame_size,pCodecCtx->sample_fmt, 1);
    frame_buf = (uint8_t *)av_malloc(size); //音频帧缓存空间
    avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt,(const uint8_t*)frame_buf, size, 1);

    //Write Header，写文件头
    avformat_write_header(pFormatCtx,NULL);

    av_new_packet(&pkt,size);

    //循环读取音频帧进行编码
    for (i=0; i<framenum; i++){
        //Read PCM，读取pcm数据到frame_buf
        if (fread(frame_buf, 1, size, in_file) <= 0){
            printf("Failed to read raw data! \n");
            return -1;
        }else if(feof(in_file)){
            break;
        }
        pFrame->data[0] = frame_buf;  //PCM Data
        pFrame->pts=i*100;
        got_frame=0;

        //Encode 编码，将pFrame中的数据编码后保存到pkt中
        ret = avcodec_encode_audio2(pCodecCtx, &pkt,pFrame, &got_frame);
        if(ret < 0){
            printf("Failed to encode!\n");
            return -1;
        }
        if (got_frame==1){
            printf("Succeed to encode 1 frame! \tsize:%5d\n",pkt.size);
            pkt.stream_index = audio_st->index;
            //将编码后的数据写入文件中
            ret = av_write_frame(pFormatCtx, &pkt);
            av_free_packet(&pkt);
        }
    }

    //Flush Encoder，最后把数据刷干净
    ret = flush_encoder(pFormatCtx,0);
    if (ret < 0) {
        printf("Flushing encoder failed\n");
        return -1;
    }

    //Write Trailer，写文件尾
    av_write_trailer(pFormatCtx);

    //Clean
    if (audio_st){
        avcodec_close(audio_st->codec);
        av_free(pFrame);
        av_free(frame_buf);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    fclose(in_file);

    return 0;
}
