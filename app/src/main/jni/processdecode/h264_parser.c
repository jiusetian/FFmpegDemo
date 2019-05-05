/**
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
} NaluType; //NALU结构类型

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIRITY_LOW = 1,
    NALU_PRIORITY_HIGH = 2,
    NALU_PRIORITY_HIGHEST = 3
} NaluPriority; //NALU优先级


typedef struct {
    //起始码长度
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    //NALU的长度
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    //
    unsigned max_size;            //! Nal Unit Buffer size
    //下面三个都是NALU中头部的信息
    int forbidden_bit;            //! should be always FALSE
    //NALU优先级
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    //NALU类型
    int nal_unit_type;            //! NALU_TYPE_xxxx
    //EBSP的指针
    char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t; //用来保存NALU信息的结构体

FILE *h264bitstream = NULL;                //!< the bit stream file

int info2 = 0, info3 = 0;

//是否为3字节起始码，返回1代表是
static int FindStartCode2(unsigned char *Buf) {
    if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1) return 0; //0x000001?
    else return 1;
}

//是否为4字节起始码
static int FindStartCode3(unsigned char *Buf) {
    if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1) return 0;//0x00000001?
    else return 1;
}

/**
 * 通过移动视频流数据中的指针，以NALU的起始码为标识找到每个NALU，并保存相关数据
 * @param nalu 是保存NALU信息的
 * @return
 */
int GetAnnexbNALU(NALU_t *nalu) {
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    //分配一个buffersize的缓存空间
    if ((Buf = (unsigned char *) calloc(nalu->max_size, sizeof(char))) == NULL)
        printf("GetAnnexbNALU: Could not allocate Buf memory\n");

    nalu->startcodeprefix_len = 3;
    //从视频流数据中读取3个字节的数据
    if (3 != fread(Buf, 1, 3, h264bitstream)) {
        free(Buf);
        return 0;
    }
    //是否为3字节起始码
    info2 = FindStartCode2(Buf);
    if (info2 != 1) { //如果不是3字节起始码
        //再读取1个字节
        if (1 != fread(Buf + 3, 1, 1, h264bitstream)) {
            free(Buf);
            return 0;
        }
        //是否为4字节起始码
        info3 = FindStartCode3(Buf);

        if (info3 != 1) { //如果也不是，则返回-1
            free(Buf);
            return -1;
        } else {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    } else {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }

    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;

    //下面主要是把指针不断往后移动，找到下一个NALU的起始码
    while (!StartCodeFound) {
        //如果到了文件流的结尾
        if (feof(h264bitstream)) {
            nalu->len = (pos - 1) - nalu->startcodeprefix_len;
            memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
            nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
            free(Buf);
            return pos - 1;
        }
        //指针往后移一个字节
        Buf[pos++] = fgetc(h264bitstream);
        //判断此时是不是一个4字节起始码
        info3 = FindStartCode3(&Buf[pos - 4]);
        //是不是一个3字节起始码
        if (info3 != 1)
            info2 = FindStartCode2(&Buf[pos - 3]);
        //如果找到下一个NALU的起始码，则退出循环
        StartCodeFound = (info2 == 1 || info3 == 1);
    }

    //找到了下一个NALU的起始码了，把指针返回去，方便下一次开始处理起始码
    // Here, we have found another start code (and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    rewind = (info3 == 1) ? -4 : -3;

    //重新把指针移回去
    if (0 != fseek(h264bitstream, rewind, SEEK_CUR)) {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }

    // Here the Start code, the complete NALU, and the next start code is in the Buf.
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

    nalu->len = (pos + rewind) - nalu->startcodeprefix_len; //当前NALU的长度
    //将NALU数据复制到buf中
    memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//复制数据
    //给NALU的头部赋值
    nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
    nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
    free(Buf); //释放Buf
    //当前NALU的大小
    return (pos + rewind);
}

/**
 * Analysis H.264 Bitstream
 * @param url    Location of input H.264 bitstream file.
 */
int simplest_h264_parser(char *url) {

    NALU_t *n;
    int buffersize = 100000;

    //FILE *myout=fopen("output_log.txt","wb+");
    //标准输出的意思，将数据写入这个文件，然后写入的数据会从屏幕上显示出来
    FILE *myout = stdout;

    //打开H264文件
    h264bitstream = fopen(url, "rb+");
    if (h264bitstream == NULL) {
        printf("Open file error\n");
        return 0;
    }

    //分配一个大小为NALU_t的内存
    n = (NALU_t *) calloc(1, sizeof(NALU_t));
    if (n == NULL) {
        printf("Alloc NALU Error\n");
        return 0;
    }

    //最大缓存大小
    n->max_size = buffersize;
    //保存EBSP数据
    n->buf = (char *) calloc(buffersize, sizeof(char));

    if (n->buf == NULL) {
        free(n);
        printf("AllocNALU: n->buf");
        return 0;
    }

    int data_offset = 0; //位置偏移到哪里了
    int nal_num = 0; //第几个NALU
    //分别是当前NALU为第几个、在整个流中的位置、权限大小、类型、NALU的长度
    printf("-----+-------- NALU Table ------+---------+\n");
    printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
    printf("-----+---------+--------+-------+---------+\n");

    //读取文件流，直到结束
    while (!feof(h264bitstream)) {
        int data_lenth; //当前NALU的长度
        //每次处理一个NALU信息，返回指针当前的位置即NALU的位置
        data_lenth = GetAnnexbNALU(n);
        //给类型赋值
        char type_str[20] = {0};
        switch (n->nal_unit_type) {
            case NALU_TYPE_SLICE:
                sprintf(type_str, "SLICE");
                break;
            case NALU_TYPE_DPA:
                sprintf(type_str, "DPA");
                break;
            case NALU_TYPE_DPB:
                sprintf(type_str, "DPB");
                break;
            case NALU_TYPE_DPC:
                sprintf(type_str, "DPC");
                break;
            case NALU_TYPE_IDR:
                sprintf(type_str, "IDR");
                break;
            case NALU_TYPE_SEI:
                sprintf(type_str, "SEI");
                break;
            case NALU_TYPE_SPS:
                sprintf(type_str, "SPS");
                break;
            case NALU_TYPE_PPS:
                sprintf(type_str, "PPS");
                break;
            case NALU_TYPE_AUD:
                sprintf(type_str, "AUD");
                break;
            case NALU_TYPE_EOSEQ:
                sprintf(type_str, "EOSEQ");
                break;
            case NALU_TYPE_EOSTREAM:
                sprintf(type_str, "EOSTREAM");
                break;
            case NALU_TYPE_FILL:
                sprintf(type_str, "FILL");
                break;
        }
        //给优先级赋值
        char idc_str[20] = {0};
        switch (n->nal_reference_idc >> 5) {
            case NALU_PRIORITY_DISPOSABLE:
                sprintf(idc_str, "DISPOS");
                break;
            case NALU_PRIRITY_LOW:
                sprintf(idc_str, "LOW");
                break;
            case NALU_PRIORITY_HIGH:
                sprintf(idc_str, "HIGH");
                break;
            case NALU_PRIORITY_HIGHEST:
                sprintf(idc_str, "HIGHEST");
                break;
        }
        //打印出结果
        fprintf(myout, "%5d| %8d| %7s| %6s| %8d|\n", nal_num, data_offset, idc_str, type_str,
                n->len);
        //NALU的位置
        data_offset = data_offset + data_lenth;
        //第几个NALU
        nal_num++;
    }

    //Free
    if (n) { //如果这个内存空间还存在的话，就释放
        if (n->buf) {
            free(n->buf);
            n->buf = NULL;
        }
        free(n);
    }
    return 0;
}
