/**
 * PCM16LE双声道数据中左声道和右声道的采样值是间隔存储的。每个采样值占用2Byte空间
 */
#include <stdio.h>
#include <malloc.h>

/*
  分离PCM16LE双声道音频采样数据的左声道和右声道
 */
int simplest_pcm16le_split(char *url) {
    //创建相关文件
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_l.pcm", "wb+");
    FILE *fp2 = fopen("output_r.pcm", "wb+");

    //分配内存空间
    unsigned char *sample = malloc(4);

    //循环读取fp文件的数据，直到结束
    while (!feof(fp)) {
        //读取4个字节到sample
        fread(sample, 1, 4, fp);
        //双声道音频中，左右声道是连续存储的，每个采样值分别为2byte
        fwrite(sample, 1, 2, fp1);
        fwrite(sample + 2, 1, 2, fp2);
    }
    //释放
    free(sample);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    return 0;
}

/*
 * 将PCM16LE双声道音频采样数据中左声道的音量降一半
 */
int simplest_pcm16le_halfvolumeleft(char *url) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_halfleft.pcm", "wb+");

    int cnt = 0;

    unsigned char *sample = (unsigned char *) malloc(4);

    while (!feof(fp)) {
        fread(sample, 1, 4, fp);
        //代表左音道
        short *sampleLeft = NULL;

        //将指针做一次强转，原来是指向一个字节的，现在short指针为指向两个字节
        sampleLeft = (short *) sample;
        //因为指向两个字节了，刚好就是一个左声道采样值，所以直接减半
        *sampleLeft = *sampleLeft / 2;

        fwrite(sample, 1, 2, fp1);
        fwrite(sample + 2, 1, 2, fp1);
    }

    //释放
    free(sample);
    fclose(fp);
    fclose(fp1);
    return 0;
}

/*
 * 将PCM16LE双声道音频采样数据的声音速度提高一倍
 */
int simplest_pcm16le_doublespeed(char *url) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_doublespeed.pcm", "wb+");

    int cnt = 0;

    unsigned char *sample = (unsigned char *) malloc(4);

    while (!feof(fp)) {
        fread(sample, 1, 4, fp);

        //只采取奇数的声道
        if (cnt % 2 != 0) {
            fwrite(sample, 1, 2, fp1);
            fwrite(sample + 2, 1, 2, fp1);
        }
        cnt++;
    }

    free(sample);
    fclose(fp);
    fclose(fp1);
    return 0;
}

/*
 * 将PCM16LE双声道音频采样数据转换为PCM8音频采样数据
 * PCM16LE格式的采样数据的取值范围是-32768到32767，而PCM8格式的采样数据的取值范围是0到255。
 * 所以PCM16LE转换到PCM8需要经过两个步骤：
 * 第一步是将-32768到32767的16bit有符号数值转换为-128到127的8bit有符号数值，
 * 第二步是将-128到127的8bit有符号数值转换为0到255的8bit无符号数值
 */
int simplest_pcm16le_to_pcm8(char *url) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_8.pcm", "wb+");

    int cnt = 0;

    unsigned char *sample = (unsigned char *) malloc(4);

    while (!feof(fp)) {
        //short类型指针
        short *samplenum16 = NULL;
        //char类型
        char samplenum8 = 0;
        //unsigned char类型
        unsigned char samplenum8_u = 0;

        //先把数据读取到缓存sample中
        fread(sample, 1, 4, fp);
        //将sample转换为两个字节的short类型指针，相当于是左声道的采样值大小
        samplenum16 = (short *) sample;
        //将16位转为八位，向右移动8个位置
        samplenum8 = (*samplenum16) >> 8;
        //再转为无符号的八位
        samplenum8_u = samplenum8 + 128;
        //将这个无符号的char数据写入到指定的文件fp1中
        fwrite(&samplenum8_u, 1, 1, fp1);

        //右声道的改变
        samplenum16 = (short *) sample + 2;
        //将16位转为八位，向右移动8个位置
        samplenum8 = (*samplenum16) >> 8;
        //再转为无符号的八位
        samplenum8_u = samplenum8 + 128;
        //将这个无符号的char数据写入到指定的文件fp1中
        fwrite(&samplenum8_u, 1, 1, fp1);

    }

    //释放
    free(sample);
    fclose(fp);
    fclose(fp1);
    return 0;
}

/**
 * 将从PCM16LE单声道音频采样数据中截取一部分数据
 */
int simplest_pcm16le_cut_singlechannel(char *url, int start_num, int dur_num) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_cut.pcm", "wb+");
    FILE *fp_stat = fopen("output_cut.txt", "wb+");

    unsigned char *sample = (unsigned char *) malloc(2);

    int cnt = 0; //用来记录读取的位置

    while (!feof(fp)) {
        //读取
        fread(sample, 1, 2, fp);

        //将指定位置范围的数据保存,每次保存一个采样值
        if (cnt > start_num && cnt <= start_num + dur_num) {

            fwrite(sample, 1, 2, fp1);

            short samplenum = sample[1];
            samplenum = samplenum * 256;
            samplenum = samplenum + sample[0];

            fprintf(fp_stat, "%6d,", samplenum);
            if (cnt % 10 == 0)
                fprintf(fp_stat, "\n", samplenum);

        }

        cnt++;
    }

    //释放
    free(sample);
    fclose(fp);
    fclose(fp1);
    fclose(fp_stat);
    return 0;
}




























